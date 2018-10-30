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
void scope_insert_global_var(scope*, char*, sexpr*);
sexpr* scope_fetch_var(vm_heap*, scope*, char*);
void env_dump(vm_heap*, scope*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0

#define CHECK_PARENT_SCOPE(vm, e, k, msg) (e->parent) \
		? scope_fetch_var(vm, e->parent, k) \
		: sexpr_err(vm, msg)


struct vm_heap {
	sexpr *heap;
	unsigned int gc_generation;
	unsigned long count;
};

vm_heap* vm_new(void);
void vm_add(vm_heap*, sexpr*);
void gc_run(vm_heap*, scope*);
void vm_free(vm_heap*);

#endif
