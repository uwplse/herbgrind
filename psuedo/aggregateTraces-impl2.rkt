#lang typed/racket

(provide abstract-traces-4)

(define-type hg-stem (U stem-leaf stem-branch))

(struct stem-leaf ([value : Number])
        #:transparent)
(struct stem-branch ([value : Number]
                     [op : Symbol]
                     [args : (Listof hg-stem)])
        #:transparent)

(: stem-value (-> hg-stem Number))
(define (stem-value stem)
 (cond
  [(stem-leaf? stem)
   (stem-leaf-value stem)]
  [(stem-branch? stem)
   (stem-branch-value stem)]))

(: tea-value (-> hg-tea (Boxof (U False Number))))
(define (tea-value tea)
  (cond
    [(tea-leaf? tea)
     (tea-leaf-value tea)]
    [(tea-branch? tea)
     (tea-branch-value tea)]))
 
(define-type concrete-expression 
  (U Number (Pairof Symbol (Listof concrete-expression))))
(define-type symbolic-expression 
  (U Number Symbol (Pairof Symbol (Listof symbolic-expression))))

(require/typed "aggregateTraces-spec.rkt"
               [precompute (-> concrete-expression Number)])

; O(n) where n is number of nodes
(: symbolic->concrete (-> symbolic-expression concrete-expression))
(define (symbolic->concrete expr)
  (cond
   [(number? expr) expr]
   [(symbol? expr) (error "Cannot convert variable to concrete expression")]
   [(list? expr) (cons (car expr) (map symbolic->concrete (cdr expr)))]))

(define-type hg-tea (U tea-leaf tea-branch))
(struct tea-leaf ([value : (Boxof (U Number False))])
        #:transparent)
(struct tea-branch ([op : Symbol] [args : (Vectorof hg-tea)]
                    [node-map : (HashTable hg-pos Integer)]
                    [value : (Boxof (U Number False))])
        #:transparent)

(define-type hg-pos (Listof Integer))

; O(n^2), where n is the number of nodes in all stems.
(: abstract-traces-4 (-> (Listof concrete-expression) symbolic-expression))
(define (abstract-traces-4 traces)
  (tea->full-expr (make-tea (map expr->stem traces))))

; O(n) where n is number of nodes
(: tea->full-expr (-> hg-tea symbolic-expression))
(define (tea->full-expr tea)
  (cond
   [(tea-leaf? tea)
    (let ([value (unbox (tea-leaf-value tea))])
      (if value 
        value
        'x))]
   [(tea-branch? tea)
    (let ([idx->var : (HashTable Integer Symbol) (make-hash)]
          [pos->idx : (HashTable hg-pos Integer) 
           (tea-branch-node-map tea)])
      (let recurse : symbolic-expression
                   ([cur-tea : hg-tea tea]
                    [cur-pos : (Listof Integer) '()])
        (cond
          [(tea-leaf? cur-tea)
           (let ([value (unbox (tea-leaf-value cur-tea))])
             (if value
               value
               (hash-ref! idx->var (hash-ref pos->idx cur-pos)
                          (λ () (gensym "x")))))]
          [(tea-branch? cur-tea)
           (cons (tea-branch-op cur-tea)
             (for/list : (Listof symbolic-expression)
                       ([arg (tea-branch-args cur-tea)]
                        [idx (in-naturals 1)])
               (recurse arg (cons idx cur-pos))))])))]))

; O(n^2) where n is number of nodes (each node might have to compute
; all nodes below it to generate intermediary values). 
(: expr->stem (-> concrete-expression hg-stem))
(define (expr->stem expr)
  (if (number? expr)
    (stem-leaf expr)
    (if (and (list? expr) (not (null? expr)) 
             (symbol? (car expr))
             (>= (length expr) 2))
      (stem-branch (precompute expr)
                   (car expr)
                   (map expr->stem (cdr expr)))
      (error "Not a valid expression!"))))

; O(n), where n is the total number of nodes in all stems.
(: make-tea (-> (Listof hg-stem) hg-tea))
(define (make-tea stems)
  (let ([tea-box (box (stem->tea (first stems)))])
    (for ([stem (rest stems)])
      (add-stem! tea-box stem))
    (unbox tea-box)))

; O(n) where n is the number of nodes in the stem.
(: stem->tea (-> hg-stem hg-tea))
(define (stem->tea stem)
  (cond
   [(stem-leaf? stem)
    (tea-leaf (box (stem-leaf-value stem)))]
   [(stem-branch? stem)
    (tea-branch (stem-branch-op stem)
                (list->vector (map stem->tea (stem-branch-args stem)))
                (get-stem-equivs stem)
                (box (stem-branch-value stem)))]))

; O(n) where n is the number of nodes in the tea.
(: generalize-structure! (-> (Boxof hg-tea) hg-stem Void))
(define (generalize-structure! tea-box stem)
  (let ([tea (unbox tea-box)])
    (cond
      [(or (stem-leaf? stem) (tea-leaf? tea)
           (not (eq? (stem-branch-op stem) (tea-branch-op tea))))
       (let ([value (unbox (tea-value tea))])
         (if (and value
                  (= value (stem-value stem)))
             (set-box! tea-box (tea-leaf (box (stem-value stem))))
             (set-box! tea-box (tea-leaf (box #f)))))]
      [else
       (when (let ([branch-value
                    (unbox (tea-branch-value tea))])
               (and branch-value
                    (not (= (stem-branch-value stem)
                            branch-value))))
         (set-box! (tea-branch-value tea) #f))
       (for ([i (in-range
                 (vector-length (tea-branch-args tea)))])
         (let ([arg-box (box (vector-ref
                              (tea-branch-args tea)
                              i))])
           (generalize-structure!
            arg-box 
            (list-ref (stem-branch-args stem) i))
           (vector-set! (tea-branch-args tea)
                        i
                        (unbox arg-box))))])))

; O(n) where n is the number of entries in the node-map.
(: get-groups (-> (HashTable hg-pos Integer) (Vectorof (Listof hg-pos))))
(define (get-groups node-map)
  (let ([groups : (Vectorof (Listof hg-pos))
         (make-vector (add1 (for/fold : Integer
                                ([max-idx : Integer 0])
                                ([(pos idx) (in-hash node-map)])
                              (max idx max-idx)))
                      '())])
    (for ([(pos idx) (in-hash node-map)])
      (vector-set! groups idx (cons pos (vector-ref groups idx))))
    groups))

; O(n1 + n2), where n1 is the number of nodes in the tea and n2 is the
; number of nodes in the stem.
(: merge-branch-node-map! (-> tea-branch stem-branch Void)) 
(define (merge-branch-node-map! tea stem)
  (let* ([tea-map-groups (get-groups (tea-branch-node-map tea))]
         [stem-map (get-stem-equivs stem)]
         [next-group-idx (vector-length tea-map-groups)])
    (for ([positions tea-map-groups] [group-idx (in-naturals)]
          #:unless (null? positions))
      (let ([split-map : (HashTable Integer Integer)
             (make-hash)])
        (hash-set! split-map (hash-ref stem-map (first positions)) group-idx)
        (for ([pos (rest positions)])
          (let* ([othervar (hash-ref stem-map pos)]
                 [existing-entry : (U Integer False)
                  (hash-ref split-map othervar #f)])
            (if existing-entry
              (when (not (= existing-entry group-idx))
                (hash-set! (tea-branch-node-map tea)
                           pos
                           existing-entry))
              (let ([new-idx next-group-idx])
                (set! next-group-idx (add1 next-group-idx))
                (hash-set! split-map othervar new-idx)
                (hash-set! (tea-branch-node-map tea)
                           pos
                           new-idx)))))))))

; O(n*d), where n is the number of entries in the given tea's
; node-map, and the d is the depth of the tea.
(: prune-map-to-structure! (-> tea-branch Void))
(define (prune-map-to-structure! tea)
  (: position-valid? (-> hg-pos Boolean))
  (define (position-valid? pos)
    (let recurse ([cur-tea : hg-tea tea] [cur-pos : hg-pos pos])
      (cond
        [(null? cur-pos) #t]
        [(tea-leaf? cur-tea)
         #f]
        [(tea-branch? cur-tea)
         (and (>= (vector-length (tea-branch-args cur-tea)) (car cur-pos))
              (recurse (vector-ref (tea-branch-args cur-tea) (sub1 (last cur-pos)))
                       (drop-right cur-pos 1)))])))
  (let ([positions (hash-keys (tea-branch-node-map tea))])
    (for ([pos positions])
      (when (not (position-valid? pos))
        (hash-remove! (tea-branch-node-map tea) pos)))))

(: add-stem! (-> (Boxof hg-tea) hg-stem Void))
(define (add-stem! tea-box stem)
  (generalize-structure! tea-box stem)
  (let ([tea (unbox tea-box)])
    (when (and (tea-branch? tea) (stem-branch? stem))
      (prune-map-to-structure! tea)
      (merge-branch-node-map! tea stem))))

; O(n) where n is the number of nodes in stem
(: get-stem-equivs (-> hg-stem (HashTable hg-pos Integer)))
(define (get-stem-equivs stem)
  (let ([var-mapping : (HashTable hg-pos Integer) (make-hash)]
        [val-mapping : (HashTable Number Integer) (make-hash)]
        [next-idx : Integer 0])
    (let recurse : Void ([cur-stem : hg-stem stem]
                         [cur-pos : (Listof Integer) '()])
      (let ([existing-var (hash-ref val-mapping
                                    (stem-value cur-stem)
                                    #f)])
       (if existing-var
         (hash-set! var-mapping cur-pos existing-var)
         (let ([new-idx next-idx])
           (set! next-idx (add1 next-idx))
           (hash-set! val-mapping (stem-value cur-stem) new-idx)
           (hash-set! var-mapping cur-pos new-idx))))
      (when (stem-branch? cur-stem)
        (for ([arg-stem (stem-branch-args cur-stem)]
              [idx (in-naturals 1)])
          (recurse arg-stem (cons idx cur-pos)))))
     var-mapping))
