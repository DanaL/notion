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
} parser;

parser* parser_create(void);
void parser_free(parser*);
void parser_clear(parser*);
void parser_feed_token(vm_heap*, parser*, token*);

#endif
