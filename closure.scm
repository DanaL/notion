; This currently doesn't work because I create and destroy the scope/hash
; tables that store variables on the fly, when I execute a user defined
; function.

; So, when I call add6 after defining it, add-factory's parameters have
; since been destroyed.
(define (addn x y) (+ x y))

(define add-factory (lambda (x)
    (lambda (y) (addn x y))
))

(define add6 (add-factory 6))
(define add4 (add-factory 4))

(define f0 addn)
(define f1 (lambda (x y)
    (* (f0 x y) (f0 x y))
))

(define make-factory (lambda (x y) (lambda () (addn x y))))

(define make-factory-alt (lambda (f0 f1) (lambda () (+ f0 f1)) ))

; This doesn't work because I am not recursing through lists in the body of
; the lambda.
(define m7  (make-factory 5 2))
(define m10 (make-factory 6 4))
(define m17 (make-factory-alt (m10) (m7)))

(define af2 (lambda (x)
    (lambda (y) (+ (+ x y)(* x y)))
))

(define make-test (lambda (test?)
    (lambda (a b) (test? a b))
))

(define test-one (lambda (test? n)
    (lambda (x) (test? x n))
))
