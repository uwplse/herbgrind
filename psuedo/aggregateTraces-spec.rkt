#lang racket
(provide flip-lists for/append
         position-get get-var-positions
         get-equalities aggregate-correct?
         all-op? precompute
         all-pequal?)

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
  (if (and (precomputable? t1) (precomputable? t2))
    (= (precompute t1) (precompute t2))
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

