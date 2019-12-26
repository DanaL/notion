; Project Euler Problem 1
(define divby3or5? (lambda (n)
    (or (= (% n 5) 0) (= (% n 3) 0))
))

; Sum all integers starting at s up to (but not including m) which are
; divisible by 3 or 5
(define euler1 (lambda (s m)
    (cond
        ((>= s m) 0)
        ((divby3or5? s) (+ s (euler1 (+ s 1) m)))
        (else (euler1 (+ s 1) m))
    )
))
