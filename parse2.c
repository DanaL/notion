#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "parser.h"
#include "sexpr.h"
#include "tokenizer.h"

void stash_some_things(tokenizer *tk) {
    /*
    token *t = token_new(T_LIST_END);
    tokenizer_stash(tk, t);
    t = token_new(T_NUM);
    t->val = "7";
    tokenizer_stash(tk, t);
    t = token_new(T_NUM);
    t->val = "2";
    tokenizer_stash(tk, t);
    t = token_new(T_NUM);
    t->val = "1";
    tokenizer_stash(tk, t);
    t = token_new(T_SYM);
    t->val = "+";
    tokenizer_stash(tk, t);
    */
    tokenizer_stash(tk, token_new(T_LIST_START));
}

int main() {
    vm_heap *vm = vm_new();
    tokenizer *tz = tokenizer_new();

    /*
    char buff[] = "+ 1 2)";
    tokenizer_feed_line(tz, buff);
    stash_some_things(tz);
    */

    //char buff[] = "(+ (* 3 4) (car (cons 1 '())) 7 5)";
    //char buff[] = "(+ (2 3 4) (5 (6 7 8)) 9 10)";
    //tokenizer_feed_line(tz, buff);

    start_file(tz, "schemer.scm");
    parser *p = parser_new(tz);

    sexpr* ast = get_next_expr(vm, p);
    while (ast->type != LVAL_NULL)
    {
        sexpr_pprint(ast);
        puts("\n\n");
        ast = get_next_expr(vm, p);
    }

    parser_free(p);

    puts("done");
}
