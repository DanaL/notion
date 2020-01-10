;; The Scheme functions to estimate a sqrt via 
;; Newton's method, as presented in section 1.1.7 of
;; SICP
(define (square x) (* x x))

(define (abs x)
    (if (< x 0)
        (- x)
        x))

(define (average x y) (/ (+ x y) 2))

(define (improve guess x)
    (average guess (/ x guess)))

(define (good-enough? guess x)
    (< (abs (- (square guess) x)) 0.001))

(define (sqrt-iter guess x)
    (if (good-enough? guess x)
        guess
        (sqrt-iter (improve guess x)
            x)))

;; notion will auto-convert between integer
;; and decimal values, which apparently MIT
;; scheme does not
(define (sqrt x)
    (sqrt-iter 1 x))
