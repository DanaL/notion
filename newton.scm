;; The Scheme functions to estimate a sqrt via 
;; Newton's method, as presented in section 1.1.7 of
;; SICP
(define (square x) (* x x))

(define (abs x)
    (if (< x 0)
        (- x)
        x))

(define (sqrt2 x)
    (sqrt-iter2 1 x))
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

;; Exercise 1.6 -- new-if throws the interpreter
;; into an infinite loop, because as a function,
;; new-if will always evaluate all of its items
(define (new-if predicate then-clause else-clause)
    (cond (predicate then-clause)
    (else else-clause)))

(define (sqrt-iter2 guess x)
    (new-if (good-enough? guess x)
        guess
        (sqrt-iter (improve guess x)
            x)))

(define (sqrt2 x)
    (sqrt-iter2 1 x))

;; Exercise 1.7. Can we improve newton's method?
;; I am passing the previous guess to compare it
;; against the new guess and looking for them to
;; almost converge

;; (sqrt 0.0001) yields 0.32308 in notion
;; (sqrt3 0.0001) yields 0.01 (which is correct)
(define (good-enough3? guess prev-guess)
    (< (abs (- 1 (/ guess prev-guess))) 0.001))

(define (sqrt-iter3 guess prev-guess x)
    (if (good-enough3? guess prev-guess)
        guess
        (sqrt-iter3 (improve guess x) guess
            x)))

(define (sqrt3 x)
    (sqrt-iter3 1 x x))

