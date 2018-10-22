#ifndef environment_h
#define environment_h

#include "fwd.h"
#include "sexpr.h"

typedef struct bucket {
	int hash_val;
	char *name;
	struct sexpr *val;
	struct bucket *next;
} bucket;

bucket* bucket_new(char*, sexpr*);
void bucket_free(bucket*);

struct scope {
	struct bucket **buckets;
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
