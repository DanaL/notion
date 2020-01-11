;; Example of a nested function definition
(define (foo x y)
  (define (bar y) (* 2 y))
  (+ 4 x (bar y))
)

(define (f x)
  (+ x 3)
  (+ x 4)
)
