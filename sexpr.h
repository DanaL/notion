#ifndef sexpr_h
#define sexpr_h

#include "fwd.h"
#include "environment.h"

enum sexpr_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_LIST, LVAL_NULL,
	LVAL_BOOL, LVAL_FUN, LVAL_STR };
enum sexpr_num_type { NUM_TYPE_INT, NUM_TYPE_DEC };

typedef sexpr*(*builtinf)(vm_heap *, scope*, sexpr**, int, char*);

struct sexpr {
	enum sexpr_type type;
	enum sexpr_num_type num_type;
	int global_scope; /* Is the value stashed in the global scope or not */

	union {
		long i_num;
		double d_num;
		int bool;
		char *sym;
		char *err;
		char *str;
	};

	int builtin;
	builtinf fun;
	sexpr *params;
	sexpr *body;

	int count;
	struct sexpr **children;
	struct sexpr *neighbour; // Used to keep track on the VM's "heap"
	unsigned int gen;
};

sexpr* sexpr_err(vm_heap*, char*);
sexpr* sexpr_num(vm_heap*, enum sexpr_num_type, double);
sexpr* sexpr_null(void);
sexpr* sexpr_sym(vm_heap*, char*);
sexpr* sexpr_list(vm_heap*);
sexpr* sexpr_bool(vm_heap*, int);
sexpr* sexpr_fun_builtin(builtinf, char*);
sexpr* sexpr_fun_user(vm_heap*, sexpr*, sexpr*, char*);
sexpr* sexpr_copy(vm_heap*, sexpr*);
sexpr* sexpr_quote(vm_heap*);
sexpr* sexpr_str(vm_heap*, char *);

void sexpr_free(sexpr*);
void sexpr_append(sexpr*, sexpr*);

char* sexpr_desc(sexpr*);
void print_sexpr_type(sexpr*);
void sexpr_pprint(sexpr*);

#define IS_ATOM(a) (a->type == LVAL_NUM || a->type == LVAL_SYM \
	|| a->type == LVAL_NULL || a->type == LVAL_BOOL \
	|| a->type == LVAL_FUN || a->type == LVAL_STR) ? 1 : 0

#define NUM_CONVERT(x) x->num_type == NUM_TYPE_INT ? x->i_num : x->d_num

#endif
