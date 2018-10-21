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
