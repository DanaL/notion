#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "parser.h"
#include "sexpr.h"
#include "tokenizer.h"

int main() {
    vm_heap *vm = vm_new();
    tokenizer *tz = tokenizer_new();

    //char buff[] = "  (car (cons 1 (quote () )) )    ";
    char buff[] = "4";
    tokenizer_feed_line(tz, buff);

    //start_file(tz, "schemer.scm");
    parser *p = parser_new(tz);
    sexpr* ast = get_next_expr(vm, p);
    sexpr_pprint(ast);
    putchar('\n');
    
    parser_free(p);

}
