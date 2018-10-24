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
