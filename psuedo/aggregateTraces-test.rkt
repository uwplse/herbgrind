
; /*-----------------------------------------------------------------------*/
; /*--- HerbGrind: a valgrind tool for Herbie  aggregateTraces-test.rkt ---*/
; /*-----------------------------------------------------------------------*/
; 
; /*
;    This file is part of HerbGrind, a valgrind tool for diagnosing
;    floating point accuracy problems in binary programs and extracting
;    problematic expressions.
; 
;    Copyright (C) 2016 Alex Sanchez-Stern
; 
;    This program is free software; you can redistribute it and/or
;    modify it under the terms of the GNU General Public License as
;    published by the Free Software Foundation; either version 3 of the
;    License, or (at your option) any later version.
; 
;    This program is distributed in the hope that it will be useful, but
;    WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;    General Public License for more details.
; 
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
;    02111-1307, USA.
; 
;    The GNU General Public License is contained in the file COPYING.
; */

#lang racket
(require "aggregateTraces-spec.rkt")
(require rackunit)

(check-true (aggregate-correct? 4 '(4 4 4)))
(check-false (aggregate-correct? 'x '(4 4 4)))
(check-true (aggregate-correct? 'x '(4 4 5)))
(check-false (aggregate-correct? 4 '(4 4 5)))
(check-true (aggregate-correct? '(+ 4 5) '((+ 4 5))))
(check-false (aggregate-correct? 'x '((+ 4 5))))
(check-false (aggregate-correct? '(- 4 5) '((+ 4 5))))
(check-false (aggregate-correct? '(+ x 5) '((+ 4 5))))
(check-true (aggregate-correct? 
            '(+ x y) 
            '((+ (+ 2 3) 4) (+ 4 5))))
(check-false (aggregate-correct? 
              'x 
              '((+ (+ 2 3) 4) (+ 4 5))))

(check-equal? (get-var-positions '(+ 1 (- x (* (+ y 3) x))))
              '((2 1) (2 2 1 1) (2 2 2)))

(check-true (aggregate-correct? '(+ x x) '((+ 1 1) (+ 2 2))))
(check-false (aggregate-correct? '(+ x x) '((+ 1 2) (+ 2 2))))
(check-false (aggregate-correct? '(+ x x) '((+ 2 2) (+ 1 2))))

(check-true (aggregate-correct? '(+ x y) '((+ 1 3) (+ 2 2))))
(check-true (aggregate-correct? '(+ y x) '((+ 2 2) (+ 1 3))))
(check-false (aggregate-correct? '(+ x y) '((+ 1 1) (+ 2 2))))

(check-true (aggregate-correct? 5 '(5 (+ 2 3))))

(check-true (aggregate-correct? '(+ 5 x) '((+ 5 5) (+ (+ 2 3) 4))))
(check-false (aggregate-correct? '(+ x x) '((+ 5 5) (+ (+ 2 3) 4))))
(check-false (aggregate-correct? '(+ y x) '((+ 5 5) (+ (+ 2 3) 4))))

(require "aggregateTraces-impl.rkt")
(require racket/random)

(define tallest-trace 4)

(define (random-trace [max-height tallest-trace] [min-height 2])
  (if (or (and (random-ref '(#f #t #t #t)) (> max-height 0))
          (> min-height 0))
    (cons (random-ref '(+ -)) (build-list 2
                                          (lambda _ (random-trace (sub1 max-height)
                                                                  (sub1 min-height)))))
    (random-ref '(1 2))))

(define (random-trace-deviation base-trace)
  (if (random-ref '(#f #t #t #t #t #t))
    (if (list? base-trace)
      (cons (car base-trace) (map random-trace-deviation (cdr base-trace)))
      base-trace)
    (random-trace tallest-trace 0)))

(define (random-traces)
  (let ([base-trace (random-trace)])
    (cons base-trace
          (for/list ([n (in-range (random 6))])
            (random-trace-deviation base-trace)))))

(for ([n (in-range 100)])
  (let ([traces (random-traces)])
    ;; (printf "Checking that:\n")
    ;; (for ([trace traces])
    ;;   (printf "~a\n" trace))
    ;; (printf "computes the correct aggregate.\n")
    (let ([computed-aggregate (abstract-traces-1 traces)])
      ;; (printf "computed as ~a\n" computed-aggregate)
      (when (not (aggregate-correct? computed-aggregate traces))
        (printf "Incorrect aggregate ~a computed for traces:\n" computed-aggregate)
        (for ([trace traces])
          (printf "~a\n" trace))
        (error "Bad aggregate")))
    ;; (printf "done!\n")))
    ))

(for ([n (in-range 100)])
  (let ([traces (random-traces)])
    ;; (printf "Checking that:\n")
    ;; (for ([trace traces])
    ;;   (printf "~a\n" trace))
    ;; (printf "computes the correct aggregate.\n")
    (let ([computed-aggregate (abstract-traces-2 traces)])
      ;; (printf "computed as ~a\n" computed-aggregate)
      (when (not (aggregate-correct? computed-aggregate traces))
        (printf "Incorrect aggregate ~a computed for traces:\n" computed-aggregate)
        (for ([trace traces])
          (printf "~a\n" trace))
        (error "Bad aggregate")))
    ;; (printf "done!\n")))
    ))

(for ([n (in-range 1000)])
  (let ([traces (random-traces)])
    ; (printf "Checking that:\n")
    ; (for ([trace traces])
    ;   (printf "~a\n" trace))
    ; (printf "computes the correct aggregate.\n")
    (let ([computed-aggregate (abstract-traces-3 traces)])
      ; (printf "computed as ~a\n" computed-aggregate)
      (when (not (aggregate-correct? computed-aggregate traces))
        (printf "Incorrect aggregate ~a computed for traces:\n" computed-aggregate)
        (for ([trace traces])
          (printf "~a\n" trace))
        (error "Bad aggregate")))
    ; (printf "done!\n")
    ))

(require "aggregateTraces-impl2.rkt")

(for ([n (in-range 100)])
  (let ([traces (random-traces)])
    ; (printf "Checking that:\n")
    ; (for ([trace traces])
    ;   (printf "~a\n" trace))
    ; (printf "computes the correct aggregate.\n")
    (let ([computed-aggregate (abstract-traces-4 traces)])
      ; (printf "computed as ~a\n" computed-aggregate)
      (when (not (aggregate-correct? computed-aggregate traces))
        (printf "Incorrect aggregate ~a computed for traces:\n" computed-aggregate)
        (for ([trace traces])
          (printf "~a\n" trace))
        (error "Bad aggregate")))
    ; (printf "done!\n")
  ))

(for ([n (in-range 100)])
  (let ([traces (random-traces)])
    ; (printf "Checking that:\n")
    ; (for ([trace traces])
    ;   (printf "~a\n" trace))
    ; (printf "computes equivalent aggregates with all algorithms.\n")
    (when (not (equal-modulo-alpha-renaming?
                (abstract-traces-1 traces)
                (abstract-traces-2 traces)
                (abstract-traces-3 traces)
                (abstract-traces-4 traces)))
      (printf "Computed aggregates are not the same! For traces:\n")
      (for ([trace traces])
        (printf "~a\n" trace))
      (printf "1: ~a\n2: ~a\n3: ~a\n4: ~a\n"
        (abstract-traces-1 traces)
        (abstract-traces-2 traces)
        (abstract-traces-3 traces)
        (abstract-traces-4 traces))
      (error "Non-matching aggregates"))
    ; (printf "done!\n")
  ))
