#ifndef parser_h
#define parser_h

#include "environment.h"
#include "sexpr.h"
#include "tokenizer.h"

sexpr* parse(char*, int*);

typedef struct parser {
    int complete;
    unsigned int open_p;
    unsigned int closed_p;
    sexpr *head;
    sexpr *curr;
    tokenizer *tk;
} parser;

parser* parser_new(tokenizer*);
void parser_free(parser*);
void parser_feed_token(vm_heap*, parser*, token*);
sexpr *get_next_expr(vm_heap *vm, parser *p);

#endif
