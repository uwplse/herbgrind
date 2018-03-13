#lang racket

(require "../../../herbie/src/formats/c.rkt")
(require "../../../herbie/src/formats/test.rkt");

(define (convert test-dir name)
  (let* ([tests (load-tests test-dir)]
         [test (car (filter (λ (test)
                               (equal? (test-name test)
                                       name))
                            tests))])
    (printf "#include <math.h>\n")
    (printf "\n")
    (printf (program->c (test-program test)))))

(module+ main
  (require racket/cmdline)

  (command-line
   #:program "fpcore-to-c.rkt"
   #:args (test-dir test-name)
   (convert test-dir test-name)))
   
