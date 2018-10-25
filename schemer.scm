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

(define member? (lambda (a lat)
    (cond
        ((null? lat) #f)
        (else (or (eq? (car lat) a) (member? a (cdr lat)) )

        )
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

(define multisubst
    (lambda (new old lat)
        (cond
            ((null? lat) '())
            (else (cond
                    ((eq? (car lat) old) (cons new (multisubst new old (cdr lat))))
                    (else (cons (car lat) (multisubst new old (cdr lat)) ))
            ))
        )
    )
)

(define insertL
    (lambda (old new lat)
        (cond
            ((null? lat) (quote ()))
            (else   (cond
                        ((eq? old (car lat))
                            (cons new lat))
                            (else (cons (car lat) (insertL old new (cdr lat)))))
))))

(define minsertL
    (lambda (old new lat)
        (cond ((null? lat) (quote ()))
        (else   (cond ((eq? old (car lat))
                    (cons new (cons old (minsertL old new (cdr lat)))   ))
                    (else (cons (car lat) (minsertL old new (cdr lat)))))
))))

(define multirember (lambda (a lat)
    (cond
        ((null? lat) (quote ()))
        (else
            (cond
                ((eq? (car lat) a) (multirember a (cdr lat)))
                (else
                    (cons (car lat) (multirember a (cdr lat)))
                )
            )
        )
    )
))

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

(define rember*
    (lambda (a l)
        (cond
            ((null? l) '())
            ((atom? (car l)) (cond ((eq? a (car l)) (rember* a (cdr l)))
                             (else (cons (car l) (rember* a (cdr l)) ))

            ))
            (else (cons (rember* a (car l)) (rember* a (cdr l))))
        )
))

(define l0 (list (list "coffee") "cup" (list (list "tea") "cup") (list "and" (list "hick")) "cup"))
(define l1 '((("tomato" "sauce")) (("bean") "sauce") ("and" (("flying")) "sauce")   )  )

(define insertR* (lambda (new old l)
    (cond
        ((null? l) '())
        ((atom? (car l))
            (cond
                ((eq? (car l) old)
                    (cons old
                        (cons new (insertR* new old (cdr l)))))
                (else (cons (car l)
                    (insertR* new old (cdr l))) )
            )
        )
    (else
        (cons (insertR* new old
            (car l)) (insertR* new old (cdr l))   ) )
    )
))

(define l2 '(("how" "much" ("wood")) "could" (("a" ("wood") "chuck")) ((("chuck"))) ("if" ("a") (("wood" "chuck"))) "could" "chuck" "wood"  ))

(define occurs* (lambda (a l)
    (cond ((null? l) 0)
        ((atom? (car l)) (cond ((eq? (car l) a)  (add1 (occurs* a (cdr l))))
                            (else (occurs* a (cdr l)))))
        (else (plus (occurs* a (car l)) (occurs* a (cdr l)))
        ))

))

(define subst* (lambda (new old l)
    (cond ((null? l) '())
            ( (atom? (car l)) (cond ((eq? (car l) old) (cons new (subst* new old (cdr l))))
                                (else (cons (car l) (subst* new old (cdr l))))
                            )
            )
            (else (cons (subst* new old (car l)) (subst* new old (cdr l))  ))
    )
))

(define insertL* (lambda (new old l)
    (cond
        ((null? l) '())
        ((atom? (car l))
            (cond
                ((eq? (car l) old)
                    (cons new
                        (cons old (insertL* new old (cdr l)))))
                (else (cons (car l)
                    (insertL* new old (cdr l))) )
            )
        )
    (else
        (cons (insertL* new old
            (car l)) (insertL* new old (cdr l))))
    )
))

(define member* (lambda (a l)
    (cond
        ((null? l) '())
        ((atom? (car l))
            (cond ((eq? (car l) a) #t)
                    (else (member* a (cdr l)))))
        (else (or (member* a (car l)) (member* a (cdr l))))
    )
))

(define leftmost (lambda (l)
    (cond
        ((null? l) #f)
        ((atom? (car l)) (car l))
        (else (leftmost (car l)))
    )
))

(define eqlist? (lambda (l1 l2)
    (cond
        ((and (null? l1) (null? l2) ) #t)
        ((or  (null? l1) (null? l2) ) #f)
        (else
            (and (equal? (car l1) (car l2))
                (equal? (cdr l1) (cdr l2))
)))))

(define equal? (lambda (s1 s2)
    (cond
        ((and (atom? s1) (atom? s2) ) (eqan? s1 s2)  )
        ((or (atom? s1) (atom? s2) ) #f)
        (else (eqlist? s1 s2))
)))

(define numbered? (lambda (aexp)
    (cond
        ((atom? aexp) (number? aexp)  )
        ((eq? (car (cdr aexp)) (quote +)) (and (numbered? (car aexp)) (numbered? (car (cdr (cdr aexp)))))  )
        ((eq? (car (cdr aexp)) (quote *)) (and (numbered? (car aexp)) (numbered? (car (cdr (cdr aexp)))))   )
        ((eq? (car (cdr aexp)) (quote ^)) (and (numbered? (car aexp)) (numbered? (car (cdr (cdr aexp)))))   )
    )
))

(define (1st-sub-exp nexp) (car (cdr nexp)))
(define (2nd-sub-exp nexp) (car (cdr (cdr nexp))))
(define (operator aexp) (car aexp))

(define value (lambda (nexp)
    (cond
        ((atom? nexp) nexp)
        ((eq? (operator nexp) (quote +)) (plus (value (1st-sub-exp nexp)) (value (2nd-sub-exp nexp)) ))
        ((eq? (operator nexp) (quote *)) (mults (value (1st-sub-exp nexp)) (value (2nd-sub-exp nexp)) ))
        ((eq? (operator nexp) (quote ^)) (^ (value (1st-sub-exp nexp)) (value (2nd-sub-exp nexp)) ))
    )
))

(define set? (lambda (lat)
    (cond
        ((null? lat) #t)
        ((member? (car lat) (cdr lat)) #f)
        (else (set? (cdr lat)))
    )
))

(define setify! (lambda (lat)
    (cond ((null? lat) (quote ()))
        (else (cons (car lat) (setify! (multirember (car lat) (cdr lat)))))
    )
))

(define subset? (lambda (set1 set2)
    (cond ((null? set1) #t) ; An empty set is always a subset I think?
        (else
            (and (member? (car set1) set2) (subset? (cdr set1) set2)  )
        )
    )
))

(define eqset? (lambda (set1 set2)
    (and (subset? set1 set2) (subset? set2 set1))
))

(define intersect? (lambda (set1 set2)
    (cond ((null? set1) #f)
        ( (member? (car set1) set2) #t)
        (else
             (intersect? (cdr set1) set2 )
        )
    )
))

(define intersect (lambda (set1 set2)
    (cond ((null? set1) (quote ()))
        (else
            (cond ((member? (car set1) set2) (cons (car set1) (intersect (cdr set1) set2))    )
                (else
                    (intersect (cdr set1) set2)))
))))

(define union (lambda (set1 set2)
    (cond ((null? set1) set2)
        ( (not (member? (car set1) set2) ) (cons (car set1) (union (cdr set1) set2)))
        (else (union (cdr set1) set2)  )
    )
))

(define intersect-all (lambda (l-set)
    (cond ((null? (cdr l-set)) (car l-set))
        (else
            (intersect (car l-set) (intersect-all (cdr l-set)))
        )
    )
))

(define a-pair? (lambda (l)
    (cond
        ((null? l) #f)
        ((atom? l) #f)
        (else (and (not (null? (cdr l))) (null? (cdr (cdr l)))))
    )
))

(define first (lambda (l) (car l)))
(define second (lambda (l) (car (cdr l))))
(define third (lambda (l) (car (cdr (cdr l)))))
(define build (lambda (s1 s2)
    (cons (first s1) (cons (second s2) (quote ()) ))
))

(define firsts (lambda (l)
    (cond
        ((null? l) (quote()))
    (else (cons (car (car l))  (firsts (cdr l)) )
    ))
))

(define fun? (lambda (rel)
    (set? (firsts rel))
))

(define revpair (lambda (p)
    (cons (second p) (cons (first p) '()))
))

(define revrel (lambda (rel)
    (cond
        ((null? rel) (quote ()))
    (else (cons (revpair (car rel)) (revrel (cdr rel)))
))))

(define seconds (lambda (l)
    (cond
        ((null? l) (quote ()))
    (else
        (cons (second (car l)) (seconds (cdr l)) )
))))

(define fullfun? (lambda (fun)
    (and (fun? fun)(set? (seconds fun)))
))
