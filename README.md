# notion
A toy scheme interpreter

I recently decided to dig into the copy of the Little Schemer I picked up ages ago and decided to work through it. 
And while I was poking around looking for a scheme to use along with it, I also discovered Daniel Holden's
Build Your Own Lisp (http://www.buildyourownlisp.com/contents) and thought...hmm I could really stand to strengthen my
C skills at the same time.

So, I decided to write my own Scheme interpreter to help my C skills while studying the Little Schemer.

This is probably a terrible way to learn either language but here we are.

I used Daniel's project to bootstrap my own, but I've diverged from his code almost from the get-go because I wanted to stick 
more closely to the dialect presented in the Little Schemer. My C is definitely way more garbage than his. It also felt like 
cheating to use his parser library so I wrote my own, crappy, recursive decent parser. Maybe one day I'll dust off my old
university notes on lex and yacc...

I was pretty tickled when, after implementing cdr, this gem ran correctly:

(/ (car (cdr (cdr(list(car (car (cdr (list 1 (list 2 3) (list (list 4)))))) 4 8)))) 2)
