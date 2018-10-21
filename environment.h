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

struct scheme_env {
	struct bucket **buckets;
	scheme_env *parent;
	unsigned int size;
};

scheme_env* env_new(unsigned int size);
void env_free(scheme_env*);
void env_insert_var(scheme_env*, char*, sexpr*);
sexpr* env_fetch_var(scheme_env*, char*);
void env_dump(scheme_env*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0

#define CHECK_PARENT_SCOPE(e, k, msg) (e->parent) \
		? env_fetch_var(e->parent, k) \
		: sexpr_err(msg)

#endif
