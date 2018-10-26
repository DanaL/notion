(define add1 (lambda (x) (+ x 1)))

(define seek (lambda (a lat)
    (cond ((null? lat) -1)
    ((eq? (car lat) a) 0)
    (else (add1 (seek a (cdr lat))))
    )
))
