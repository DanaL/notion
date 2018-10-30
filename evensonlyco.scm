(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
))

(define even?
    (lambda (n)
        (= (* (/ n 2) 2) n)
))

(define evens-only*
    (lambda (l)
        (cond
            ((null? l) (quote ()))
            ((atom? (car l))
                (cond
                    ((even? (car l)) (cons (car l) (evens-only* (cdr l)))    )
                    (else (evens-only* (cdr l)))))
            (else
                (cons (evens-only* (car l)) (evens-only* (cdr l)))

        ))
))

(define the-last-friend
    (lambda (newl product sum)
        (cons sum (cons product newl))
    )
)

(define evens-only*&co
    (lambda (l col)
        (cond
            ((null? l) (col '() 1 0))
            ((atom? (car l))
                (cond
                     ; atom, dealing with even
                        ((even? (car l))
                            (evens-only*&co (cdr l)
                                (lambda (newl p s)
                                    (col (cons (car l) newl) (* (car l) p s))
                                )
                        )
                        ; otherwise deal with odd
                        (else (evens-only*&co (cdr l)
                            (lambda (newl p s)
                                (col (newl p (+ (car l s))))
                            )
                        ))
            ;)))
            ;(else
            ;     'FUCK
            ;)
))))))

(define evens-only&co
    (lambda (l col)
        (cond
            ((null? l) (col '() 1 0))
            ((atom? (car l))
                (cond
                    ((even? (car l))
                        (evens-only*&co2 (cdr l)
                            (lambda (newl p s)
                                (col (cons (car l) newl) (* (car l) p) s)
                    )))
                    (else
                        (evens-only*&co2 (cdr l)
                            (lambda (newl p s)
                                (col newl p (+ (car l) s) )
                    )))
                )
            )
        )
))

; Stolen from https://github.com/willprice/little-schemer/blob/master/collectors.rkt
; Because the scond else statement was beyond me
(define evens-only*&co3
  (lambda (l col)
    (cond
      ((null? l)
       (col '() 1 0))
      ((atom? (car l))
       (cond
         ((even? (car l))
          (evens-only*&co3
            (cdr l)
            (lambda (evens-l evens-product odds-sum)
              (col (cons (car l)
                         evens-l)
                   (* (car l)
                      evens-product)
                   odds-sum))))
         (else
           (evens-only*&co3
             (cdr l)
             (lambda (evens-l evens-product odds-sum)
               (col evens-l
                    evens-product
                    (+ (car l)
                       odds-sum)))))))
      (else
        ; The idea here is that we recurse into the car, and our
        ; collector handles recursing into the cdr. The cdr collector
        ; then combines the result as it has access to the results for
        ; both the car and cdr
        (evens-only*&co3
          (car l)
          (lambda (car-evens-l car-evens-product car-odds-sum)
            (evens-only*&co3
              (cdr l)
              (lambda (cdr-evens-l cdr-evens-product cdr-odds-sum)
                (col
                  (cons car-evens-l cdr-evens-l)
                  (* car-evens-product cdr-evens-product)
                  (+ car-odds-sum cdr-odds-sum))))))))))
