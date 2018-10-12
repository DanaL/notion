#ifndef parser_h
#define parser_h

enum token_type { T_NUM, T_SYM, T_STR, T_NULL, T_LIST_START, T_LIST_END, T_UNKNOWN };

typedef struct token {
	enum token_type type;
	char *val;
} token;

token* token_create(enum token_type);
void token_free(token *);
void dump_tokens(char *);

enum sexpr_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_LIST, LVAL_NULL };
enum sexpr_num_type { NUM_TYPE_INT, NUM_TYPE_DEC };

typedef struct sexpr {
	enum sexpr_type type;
	enum sexpr_num_type num_type;
	union {
		long i_num;
		float d_num;
	} n;
	char *sym;
	char *err;
	int count;
	struct sexpr **children;
} sexpr;

sexpr* sexpr_err(char*);
sexpr* sexpr_num_s(char*);
sexpr* sexpr_num(sexpr*);
sexpr* sexpr_null(void);
sexpr* sexpr_sym(char*);
sexpr* sexpr_list(void);
sexpr* sexpr_null(void);

sexpr* sexpr_copy(sexpr*);
void sexpr_free(sexpr*);
void sexpr_list_insert(sexpr*, sexpr*);
void sexpr_append(sexpr*, sexpr*);

void sexpr_pprint(sexpr*, int);

sexpr* parse(char*, int*);

#define IS_ATOM(a) (a->type == LVAL_NUM || a->type == LVAL_SYM || a->type == LVAL_NULL) ? 1 : 0

#endif
