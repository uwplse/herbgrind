;; #!/usr/bin/racket
;; #lang racket

(define (flip-lists list-list)
  (apply map list list-list))

(define-syntax-rule (for/append (defs ...)
                                bodies ...)
  (apply append
         (for/list (defs ...)
           bodies ...)))

(define (all-pequal? const traces)
  (andmap (curry pequal? const) traces))

(define (all-same-pequal? traces)
  (all-pequal? (precompute (first traces)) traces))

(define (all-op? op traces)
  (and (andmap list? traces)
       (andmap (curry eq? op)
               (map first traces))))

(define (all-same-op? traces)
  (or (null? traces)
      (and (list? (first traces))
           (all-op? (first (first traces)) traces))))

(define (structure-matches? aggr traces)
  (match aggr
    [`(,op . ,args) 
     (and (all-op? op traces)
          (for/and ([arg args] [trace-args (flip-lists (map rest traces))])
            (structure-matches? arg trace-args)))]
    [(? number?)
     (and (not (all-same-op? traces))
          (all-pequal? aggr traces))]
    [(? symbol?)
      (and (not (all-same-op? traces))
           (not (all-same-pequal? traces)))]))

(define (valid-pos? pos tree)
  (if (null? pos) #t
    (match pos
      [`(,idx . ,rest)
       (and (number? idx)
            (list? tree)
            (> (length tree) idx)
            (valid-pos? rest (list-ref tree idx)))]
      [else #f])))

(define ps (make-base-namespace))
(define (precompute trace)
  (eval trace ps))

(define (position-get pos tree)
  (if (null? pos)
    tree
    (match pos
      [`(,idx . ,rest)
       (position-get rest (list-ref tree idx))]
      [else #f])))

(define (get-var-positions aggr)
  (match aggr
    [`(,op . ,args)
     (for/append ([arg args] [idx (in-naturals 1)])
       (let ([inner (get-var-positions arg)])
         (for/list ([var-pos inner])
           (cons idx var-pos))))]
    [(? number?)
     '()]
    [(? symbol?)
     '(())]))

(define (precomputable? tree)
  (null? (get-var-positions tree)))

(define (pequal? t1 t2)
  (if (or (number? t1) (number? t2))
      (and (precomputable? t1) (precomputable? t2)
           (= (precompute t1) (precompute t2)))
      (match t1
        [`(,op . ,args)
          (and (list? t1)
               (equal? op (first t2))
               (= (length args) (length (rest t2)))
               (for/and ([arg args] [arg2 (rest t2)])
                 (pequal? arg arg2)))]
        [(? symbol?)
         (equal? t1 t2)])))

(define (get-possible-equalities positions)
  (for/append ([pos1 positions])
    (for/list ([pos2 (remove pos1 positions)])
      `(= ,pos1 ,pos2))))

(define (equality-valid? equality tree)
  (match equality
    [`(= ,pos1 ,pos2)
     (when (not (valid-pos? pos1 tree))
       (error "Invalid position" pos1 tree))
     (when (not (valid-pos? pos2 tree))
       (error "Invalid position" pos2 tree))
     (pequal? (position-get pos1 tree)
              (position-get pos2 tree))]
    [else #f]))

(define (get-equalities positions tree)
  (filter (curryr equality-valid? tree)
          (get-possible-equalities positions)))

(define (get-all-positions tree)
  (match tree
    [`(,op . ,args)
     (cons '()
           (for/append ([arg args] [idx (in-naturals 1)])
             (map (curry cons idx) (get-all-positions arg))))]
    [(or (? number?) (? symbol?))
     '(())]
    [else '()]))

(define (get-all-equalities tree)
  (get-equalities (get-all-positions tree) tree))

;; Determine whether every equality that holds between variables in the
;; aggregate also holds in every trace.

(define (var-map-sound? aggr traces)
  (let ([var-equalities (get-equalities (get-var-positions aggr) aggr)])
    (for/and ([var-equality var-equalities])
      (for/and ([trace traces])
        (equality-valid? var-equality trace)))))

;; Determine whether every equality that holds in every trace holds in
;; the aggregate.

(define (var-map-complete? aggr traces)
  (let ([universal-equalities 
         (if (null? traces) '()
           (apply set-intersect (map (curry get-equalities (get-var-positions aggr))
                                     traces)))])
    (for/and ([equality universal-equalities])
      (equality-valid? equality aggr))))

(define (aggregate-correct? aggr traces)
  (and (structure-matches? aggr traces)
       (var-map-sound? aggr traces)
       (var-map-complete? aggr traces)))

;; TODO: Fill these in.

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

;; (define (abstract-traces-2 traces)
;;   (let* ([structure (get-structure-with-consts traces)]
;;          [var-positions (get-var-positions structure)]
;;          [var-mapping (for/fold ([var-mapping (get-equiv-mapping var-positions (first traces))])
;;                                 ([trace (rest traces)])
;;                         (merge-var-mappings var-mapping (get-equiv-mapping var-positions trace)))])
;;     (assign-variables-from-map structure var-mapping)))
;; 
;; (define (generalize-aggr aggr trace)
;;   (let* ([structure (generalize-structure aggr trace)]
;;          [var-mapping (merge-var-mappings (get-var-mapping aggr)
;;                                           (get-equiv-mapping (var-positions aggr) trace))])
;;     (assign-variables-from-map structure var-mapping)))

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
