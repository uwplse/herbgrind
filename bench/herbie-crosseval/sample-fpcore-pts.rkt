#lang racket

(require math/bigfloat)
(require "../../../herbie/src/formats/test.rkt");
(require "../../../herbie/src/points.rkt")
(require "../../../herbie/src/config.rkt")
(require "../../../herbie/src/common.rkt")

(define (get-points test-dir name)
  (let* ([tests (load-tests test-dir)]
         [test (car (filter (λ (test)
                              (equal? (test-name test)
                                      name))
                            tests))]
         [context (prepare-points
                   (test-program test)
                   (test-samplers test)
                   '#t)])
    (printf "#define PRECISION ~a\n" (bf-precision))
    (printf "double pts[~a][NARGS] = {\n"
            (*num-points*))
    (for ([(pt ex) (in-pcontext context)])
      (printf "    {")
      (for ([dim pt])
        (printf "~a, " dim))
      (printf "},\n"))
    (printf "};\n")))
              
    

(module+ main
  (require racket/cmdline)

  (command-line
   #:program "sample-fpcore-pts.rkt"
   #:once-each
   [("-n") npts "Number of points."
    (*num-points* (string->number npts))]
   [("-r") seed "Random seed."
    (define given-seed (read (open-input-string seed)))
    (when given-seed (set-seed! given-seed))]
   #:args (test-dir test-name)
   (get-points test-dir test-name)))
