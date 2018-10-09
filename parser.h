#ifndef parser_h
#define parser_h

enum token_type { T_OP, T_NUM, T_STR, T_NULL, T_SEXPR_START, T_SEXPR_END };

typedef struct token {
	enum token_type type;
	char *val;
} token;

token* token_create(enum token_type, char*);
void token_free(token *);

enum lval_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };
enum lval_num_type { NUM_TYPE_INT, NUM_TYPE_DEC };

typedef struct lval {
	enum lval_type type;
	enum lval_num_type num_type;
	union {
		long i_num;
		float d_num;
	} n;
	char *sym;
	char *err;
	int count;
	struct lval **children;
} lval;

lval* lval_err(char*);
lval* lval_num(char*);
lval* lval_null(void);
lval* lval_sym(char*);
lval* lval_sexpr(void);
void lval_free(lval*);
lval* lval_append(lval*, lval*); 

void lval_pprint(lval*, int);

lval* parse(char *s);

#endif