#ifndef environment_h
#define environment_h

#include "parser.h"

#define TABLE_SIZE 1019

typedef struct bucket {
	int hash_val;
	struct sexpr *val;
	struct bucket *next;
} bucket;

bucket* bucket_new(sexpr*);
void bucket_free(bucket*);

typedef struct scheme_env {
	struct bucket **buckets;
} scheme_env;

scheme_env* env_new(void);
void env_free(scheme_env*);
void env_insert_var(scheme_env*, sexpr*);
sexpr* env_fetch_var(char*);

#endif