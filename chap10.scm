(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
))

(define first (lambda (l) (car l)))
(define second (lambda (l) (car (cdr l))))

(define member? (lambda (a lat)
    (cond
        ((null? lat) #f)
        (else (or (eq? (car lat) a) (member? a (cdr lat)) )

        )
    )
))

(define set? (lambda (lat)
    (cond
        ((null? lat) #t)
        ((member? (car lat) (cdr lat)) #f)
        (else (set? (cdr lat)))
    )
))

(define lookup-in-entry-help
    (lambda (name names values entry-f)
        (cond
            ((null? names) (entry-f name))
            ((eq? (car names) name) (car values))
            (else
                (lookup-in-entry-help name (cdr names) (cdr values) entry-f)
            )
)))

(define lookup-in-entry
    (lambda (name entry entry-f)
        (lookup-in-entry-help name
            (first entry)
            (second entry)
            entry-f
        )
))

(define found (lambda (name) (not #t)))

(define extend-table (lambda (entry table)
    (cons entry table)
))

(define entry '(("app" "entree" "beverage") ("pate" ("boeuf" "pasta") ("beer" "vin"))))

(define table '((("entree" "dessert") ("spaghetti" "spumoni")) (("appertizer" "entree" "beverage") ("food" "tastes" "good"))))

(define lookup-in-table (lambda (name table table-f)
    (cond
        ((null? table) (table-f name))
        (else (lookup-in-entry name (car table)
                (lambda (name)
                    (lookup-in-table name (cdr table) table-f)
                )
            )
        )
    )
))
