; I need to do more exercises like chapter 8 to better understand the whole
; collector function pattern

(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
))

; first, a function that filters out any numbers less than min or greater
; than max
(define middling
    (lambda (min max lat)
        (cond
            ((null? lat)  (quote ()))
            ((> (car lat) max) (middling min max (cdr lat)))
            ((< (car lat) min) (middling min max (cdr lat)))
            (else
                (cons (car lat) (middling min max (cdr lat)))
            )
        )
))

(define collector
    (lambda (newl lt gt)
        (cons lt (cons gt newl))
    )
)

; Can I also make a collector function work??
(define middling&co
    (lambda (min max lat col)
        (cond
            ((null? lat) (col '() 0 0))
            ; Case where head is greater than max
            ( (> (car lat) max)
                    (middling&co min max (cdr lat)
                        (lambda (newl lt gt)
                            (col newl lt (+ 1 gt))
                        )
                    )
            )

            ( (< (car lat) min)
                    (middling&co min max (cdr lat)
                        (lambda (newl lt gt)
                            (col newl (+ 1 lt) gt)
                        )
                    )
            )
            (else
                (cons (car lat) (middling&co min max (cdr lat) col))
            )
        )
))
