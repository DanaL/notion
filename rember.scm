(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
))

(define rem-fact
    (lambda (test?)
    (lambda (a l)
        (cond
            ((null? l) '())
            ((atom? (car l)) (cond ((test? a (car l)) ((rem-fact test?) a (cdr l)))
                             (else (cons (car l) ((rem-fact test?) a (cdr l)) ))

            ))
            (else (cons ((rem-fact test?) a (car l)) ((rem-fact test?) a (cdr l))))
        )
)))

; Generates functions that remove all items in list that are > n
(define rem-gt-n
    (lambda (n)
        (lambda (l) ((rem-fact <) n l))
))
