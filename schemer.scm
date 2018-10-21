(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
))

(define lat? (lambda (l)
    (cond (
            (null? l) #t)
            ((atom? (car l)) (lat? (cdr l)))
            (else #f)
        )
))

(define insertR (lambda (old new lat)
    (cond
        ((null? lat) (quote ()))
        (else
            (cond
                ((eq? old (car lat)) (cons (car lat) (cons new (cdr lat))))
                (else (cons (car lat) (insertR old new (cdr lat)))))
))))

; Chapter 4 has us re-implementing basic arithmetic :P
(define add1 (lambda (x) (+ x 1)))
(define sub1 (lambda (x) (- x 1)))

(define plus (lambda (x y)
    (cond ((eq? y 0) x)
    (else (plus (add1 x) (sub1 y)))
 )))

(define addtup (lambda (tup)
  (cond ((null? tup) 0)
  (else (plus (car tup) (addtup (cdr tup))))
)))

; add two tuples of the same length
(define tup+ (lambda (tup1 tup2)
    (cond
      ((and (null? tup1)(null? tup2)) (quote ())
     (else (cons (plus (car tup1)(car tup2))(tup+ (cdr tup1)(cdr tup2)))) )))
)

(define mults (lambda (x y)
  (cond ((eq? y 0) 0)
  (else (plus x (mults x (sub1 y)) ) )
)))

; Alternatively, I could do mults by generating a tuple and then sending
; it to addtup
(define maketup (lambda (x n)
     (cond ((eq? n 0) '() )
     (else (cons x (maketup x (sub1 n)))))
))

(define mults-alt (lambda (x y) (addtup (maketup x y)) ))

(define gt (lambda (x y)
    (cond ((eq? x 0) #f)
            ((eq? y 0) #t)
            (else (gt (sub1 x)(sub1 y)))
    )
))

(define lt (lambda (x y)
    (cond ((eq? y 0) #f)
            ((eq? x 0) #t)
            (else (lt (sub1 x)(sub1 y)))
    )
))

(define eqq (lambda (x y)
    (cond  ( (and (not (gt x y)) (and (not (lt x y))) ) #t )
            (else #f)
    )
))

(define pow (lambda (x y)
    (cond ((eq? y 0) 1)
            (else (mults x (pow x (sub1 y) )))
    )
))

(define length (lambda (l)
    (cond  ((null? l)   0)
            (else (add1 (length (cdr l))   ))
    )
))

(define pick (lambda (l n)
    (cond ((null? l)   '())
            ((eq? (sub1 n) 0) (car l))
            (else (pick (cdr l) (sub1 n)))
    )
))


(define no-nums (lambda (l)
    (cond ((null? l)  '() )
    (else (cond
            ((number? (car l)) (no-nums (cdr l)))
            (else (cons (car l) (no-nums (cdr l))  ))
        )
    ))
))

(define all-nums (lambda (l)
    (cond ((null? l)  '() )
    (else (cond
            ((number? (car l)) (cons (car l) (all-nums (cdr l))))
            (else  (all-nums (cdr l)))
        )
    ))
))

; A bit redundant because I wrote my eq? to work on numbers since a couple
; of scheme implementations I tried also did it that way
(define eqan? (lambda (a b)
    (cond ( (and (number? a)(number? b)) (= a b) )
        ((or (number? a) (number? b)) #f)
        (else (eq? a b))
    )
))

(define occurs (lambda (a l)
    (cond ((null? l) 0)
            (else (cond
                    ((eqan? a (car l)) (add1 (occurs a (cdr l)) ))
                    (else (occurs a (cdr l)))
            ))
    )
))

(define one? (lambda (n) (eq? n 1)))

(define rempick (lambda (l n)
    (cond ((null? l)   '())
            ((one? n) (cdr l))
            (else (cons (car l) (rempick (cdr l) (sub1 n))))
    )
))
