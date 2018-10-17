#ifndef parser_h
#define parser_h

#include "sexpr.h"

enum token_type { T_NUM, T_SYM, T_STR, T_NULL, T_LIST_START, T_LIST_END,
	T_UNKNOWN, T_CONSTANT, T_SINGLE_QUOTE, T_ERR };

typedef struct token {
	enum token_type type;
	char *val;
} token;

token* token_create(enum token_type);
void token_free(token *);
void dump_tokens(char *);

sexpr* parse(char*, int*);

#endif
