#ifndef environment_h
#define environment_h

#include "fwd.h"
#include "sexpr.h"

typedef struct sym {
	int hash_val;
	char *name;
	struct sexpr *val;
	struct sym *next;
} sym;

sym* sym_new(char*, sexpr*);
void sym_free(sym*);

/* Not sure if scope or sym_table will be a better name for this in the end */
struct scope {
	struct sym **sym_table;
	scope *parent;
	unsigned int size;
};

scope* scope_new(unsigned int size);
void scope_free(scope*);
void scope_insert_var(scope*, char*, sexpr*);
sexpr* scope_fetch_var(scope*, char*);
void env_dump(scope*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0

#define CHECK_PARENT_SCOPE(e, k, msg) (e->parent) \
		? scope_fetch_var(e->parent, k) \
		: sexpr_err(msg)

#endif
