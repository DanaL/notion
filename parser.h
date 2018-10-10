#ifndef parser_h
#define parser_h

enum token_type { T_OP, T_NUM, T_STR, T_NULL, T_SEXPR_START, T_SEXPR_END };

typedef struct token {
	enum token_type type;
	char *val;
} token;

token* token_create(enum token_type, char*);
void token_free(token *);

enum lval_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_LIST };
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
lval* lval_num_s(char*);
lval* lval_num(lval*);
lval* lval_null(void);
lval* lval_sym(char*);
lval* lval_sexpr(void);
lval* lval_list(void);
int lval_is_atom(lval*);
lval* lval_copy_atom(lval*);
void lval_copy_list(lval*, lval*);
void lval_free(lval*);
void lval_list_insert(lval*, lval*);
void lval_append(lval*, lval*); 

void lval_pprint(lval*, int);

lval* parse(char *s);

#endif