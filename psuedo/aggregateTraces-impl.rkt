
; /*--------------------------------------------------------------------*/
; /*--- HerbGrind: a valgrind tool for Herbie            hg_opinfo.c ---*/
; /*--------------------------------------------------------------------*/
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

(provide abstract-traces-1
         abstract-traces-2
         generalize-aggr
         abstract-traces-3)

(require "aggregateTraces-spec.rkt")
(require racket/hash)

(define (get-abstract-structure traces)
  (if (and (andmap list? traces) (all-op? (car (first traces)) traces))
    (cons (car (first traces))
          (for/list ([child-traces (flip-lists (map rest traces))])
            (get-abstract-structure child-traces)))
    'x))

(define (trim-to-structure structure trace)
  (if (list? structure)
    (cons (car structure) (map trim-to-structure (rest structure) (rest trace)))
    (precompute trace)))

(define (fill-consts structure traces)
  (if (list? structure)
    (cons (car structure) (map fill-consts (rest structure) (flip-lists (map rest traces))))
    (let ([pfirst (precompute (first traces))])
      (if (all-pequal? pfirst traces)
        pfirst
        'x))))

(define (assign-variables-from-equalities structure-with-consts var-equalities)
  (define (equal-positions position equalities)
    (remove-duplicates
      (for/list ([equality equalities]
                 #:when (member position (rest equality)))
        (if (equal? (cadr equality) position)
          (caddr equality)
          (cadr equality)))))
  (define (get-existing-var-sym positions mapping)
    (for/or ([pos positions])
      (hash-ref mapping pos #f)))
  (let ([pos-var-mapping (make-hash)])
    (for ([var-pos (get-var-positions structure-with-consts)])
      (let* ([eq-positions (equal-positions var-pos var-equalities)]
             [var-sym (get-existing-var-sym eq-positions pos-var-mapping)])
        (if var-sym
          (hash-set! pos-var-mapping var-pos var-sym)
          (let ([newsym (gensym "x")])
            (hash-set! pos-var-mapping var-pos newsym)
            newsym))))
    (assign-variables-from-map structure-with-consts pos-var-mapping)))

(define (assign-variables-from-map structure mapping)
  (let recurse ([structure structure] [cur-pos '()])
    (let ([var (hash-ref mapping cur-pos #f)])
      (if var var
        (if (list? structure)
          (cons (car structure)
                (for/list ([child (cdr structure)] [idx (in-naturals 1)])
                  (recurse child (append cur-pos (list idx)))))
          structure)))))

(define (abstract-traces-1 traces)
  (let* ([structure (get-abstract-structure traces)]
         [trimmed-traces (map (curry trim-to-structure structure) traces)]
         [structure-with-consts (fill-consts structure traces)]
         [var-equalities (apply set-intersect (map (curry get-equalities 
                                                          (get-var-positions structure-with-consts))
                                                   traces))])
    (assign-variables-from-equalities structure-with-consts var-equalities)))

(define (get-structure-with-consts traces)
  (if (and (andmap list? traces)
           (all-op? (car (first traces)) traces))
    (cons (car (first traces))
          (for/list ([child-traces (flip-lists (map rest traces))])
            (get-structure-with-consts child-traces)))
    (let ([const (precompute (first traces))])
      (if (all-pequal? const traces)
          const
          'x))))

(define (get-equiv-mapping positions trace)
  (let ([var-mapping (make-hash)]
        [val-mapping (make-hash)])
    (for ([pos positions])
      (let* ([val (precompute (position-get pos trace))]
             [existing-var (hash-ref val-mapping val #f)])
        (if existing-var
          (hash-set! var-mapping pos existing-var)
          (let ([new-var (gensym "x")])
            (hash-set! val-mapping val new-var)
            (hash-set! var-mapping pos new-var)))))
    var-mapping))

(define (reverse-var-mapping mapping)
  (let ([reverse-mapping (make-hash)])
    (for ([(k v) (in-hash mapping)])
      (hash-update!
        reverse-mapping
        v
        (curry cons k)
        '()))
    reverse-mapping))

(define (merge-var-mappings mapping1 mapping2)
  (let* ([rmapping1 (reverse-var-mapping mapping1)]
         [result (make-hash)])
    (for ([(var positions) (in-hash rmapping1)])
      (let ([split-map (make-hash)]
            [group-othervar #f])
        (for ([pos positions])
          (let ([othervar (hash-ref mapping2 pos)])
            (when (not group-othervar)
              (set! group-othervar othervar)
              (hash-set! split-map othervar var))
            (let ([existing-entry 
                   (hash-ref split-map othervar #f)])
              (if existing-entry
                (hash-set! result pos existing-entry)
                (let ([newvar (gensym "x")])
                  (hash-set! split-map othervar newvar)
                  (hash-set! result pos newvar))))))))
    result))

(define (abstract-traces-2 traces)
  (let* ([structure (get-structure-with-consts traces)]
         [var-positions (get-var-positions structure)]
         [var-mapping (for/fold ([var-mapping
                                  (get-equiv-mapping 
                                   var-positions 
                                   (first traces))])
                                ([trace (rest traces)])
                        (merge-var-mappings
                           var-mapping
                           (get-equiv-mapping var-positions 
                                              trace)))])
    (assign-variables-from-map structure var-mapping)))

(define (generalize-structure aggr-expr trace)
  (match aggr-expr
    [(? number?)
     (if (= aggr-expr (precompute trace))
       aggr-expr
       'x)]
    [(? symbol?)
     aggr-expr]
    [`(,op . ,args)
     (if (and (list? trace) (eq? (car trace) op))
       (cons op (map generalize-structure args (cdr trace)))
       (if (and (precomputable? aggr-expr) (= (precompute aggr-expr) (precompute trace)))
         (precompute aggr-expr)
         'x))]))

(define (get-var-mapping aggr)
  (let recurse ([aggr aggr] [pos '()])
    (match aggr
     [(? number?)
      (hash)]
     [(? symbol?)
      (hash pos aggr)]
     [`(,op . ,args)
      (apply hash-union
       (for/list ([arg args] [idx (in-naturals 1)])
         (recurse arg (append pos (list idx)))))])))

(struct aggregate0 (node-equiv-map node-const-map expr) #:transparent)

(define (get-const-mapping positions stem)
  (for/hash ([pos positions])
    (values pos (precompute (position-get pos stem)))))

(define (merge-const-mappings mapping1 mapping2)
  (for/hash ([(pos1 val1) (in-hash mapping1)]
             #:when (= val1 (hash-ref mapping2 pos1 #f)))
    (values pos1 val1)))
(define (trace->aggregate0 trace)
  (aggregate0 (get-equiv-mapping (get-all-positions trace) trace)
              (get-const-mapping (get-all-positions trace) trace)
              trace))

(define (aggregate0->full-expr aggr)
  (assign-constants-from-map
    (assign-variables-from-map 
      (aggregate0-expr aggr)
      (prune-pos-map (aggregate0-node-equiv-map aggr)
                     (get-var-positions (aggregate0-expr aggr))))
    (prune-pos-map (aggregate0-node-const-map aggr)
                   (get-var-positions (aggregate0-expr aggr)))))

(define (assign-constants-from-map tea-expr mapping)
  (let recurse ([cur-pos '()] [cur-expr tea-expr])
    (let ([constant (hash-ref mapping cur-pos #f)])
      (if constant constant
        (if (list? cur-expr)
          (cons (car cur-expr)
                (for/list ([subexpr (cdr cur-expr)] [idx (in-naturals 1)])
                  (recurse (cons idx cur-pos) subexpr)))
          cur-expr)))))

(define (prune-pos-map pos-map positions)
  (for/hash ([pos positions]
             #:when (hash-ref pos-map pos #f))
    (values pos (hash-ref pos-map pos))))

(define (generalize-aggr aggr trace)
  (let* ([structure (generalize-structure (aggregate0-expr aggr) trace)]
         [var-mapping (merge-var-mappings (prune-pos-map
                                            (aggregate0-node-equiv-map aggr)
                                            (get-all-positions structure))
                                          (get-equiv-mapping (get-all-positions structure)
                                                             trace))]
         [const-mapping (merge-const-mappings (prune-pos-map
                                                (aggregate0-node-const-map aggr)
                                                (get-all-positions structure))
                                              (get-const-mapping (get-all-positions structure)
                                                                 trace))])
    (aggregate0 var-mapping const-mapping structure)))

(define (abstract-traces-3 traces)
  (aggregate0->full-expr
    (foldl (lambda (trace aggr) (generalize-aggr aggr trace))
           (trace->aggregate0 (first traces))
           (rest traces))))
