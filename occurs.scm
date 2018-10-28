(define add1 (lambda (x) (+ x 1)))

(define occurs (lambda (a l)
    (cond ((null? l) 0)
            (else (cond
                    ((eq? a (car l)) (add1 (occurs a (cdr l)) ))
                    (else (occurs a (cdr l)))
            ))
    )
))
