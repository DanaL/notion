#ifndef sexpr_h
#define sexpr_h

#include "fwd.h"
#include "environment.h"

enum sexpr_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_LIST, LVAL_NULL,
	LVAL_BOOL, LVAL_FUN, LVAL_STR };
enum sexpr_num_type { NUM_TYPE_INT, NUM_TYPE_DEC };

typedef sexpr*(*builtinf)(scheme_env*, sexpr**, int, char*);

struct sexpr {
	enum sexpr_type type;
	enum sexpr_num_type num_type;

	union {
		long i_num;
		float d_num;
	} n;
	int bool;
	char *sym;
	char *err;
	char *str;

	int builtin;
	builtinf fun;
	sexpr *params;
	sexpr *body;

	int count;
	struct sexpr **children;

	struct sexpr *parent;
};

sexpr* sexpr_err(char*);
sexpr* sexpr_num_s(char*);
sexpr* sexpr_num(enum sexpr_num_type, float);
sexpr* sexpr_null(void);
sexpr* sexpr_sym(char*);
sexpr* sexpr_list(void);
sexpr* sexpr_null(void);
sexpr* sexpr_bool(int);
sexpr* sexpr_fun_builtin(builtinf, char*);
sexpr* sexpr_fun_user(sexpr*, sexpr*, char*);
sexpr* sexpr_copy(sexpr*);
sexpr* sexpr_quote(void);
sexpr* sexpr_str(char *);

void sexpr_free(sexpr*);
void sexpr_list_insert(sexpr*, sexpr*);
void sexpr_append(sexpr*, sexpr*);

void print_sexpr_type(sexpr*);
void sexpr_pprint(sexpr*);

#define IS_ATOM(a) (a->type == LVAL_NUM || a->type == LVAL_SYM \
	|| a->type == LVAL_NULL || a->type == LVAL_BOOL \
	|| a->type == LVAL_FUN || a->type == LVAL_STR) ? 1 : 0

#define NUM_CONVERT(x) x->num_type == NUM_TYPE_INT ? x->n.i_num : x->n.d_num

#endif
