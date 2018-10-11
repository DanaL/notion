#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "parser.h"
#include "environment.h"

bucket* bucket_new(sexpr* e) {
	bucket *b = malloc(sizeof(bucket));
	b->val = e;

	return b;
}

void bucket_free(bucket* b) {

}

scheme_env* env_new(void) {
	scheme_env *e = malloc(sizeof(scheme_env));
	bucket **buckets[TABLE_SIZE] = { NULL };
	e->buckets = buckets;

	return e;
}

int bt_hash(char *s) {
	long h = 0;
	int len = strlen(s);

	for (int j = 0; j < len; j++) {
		h += (long)pow(163, len - (j+1)) * s[j];
		h %= TABLE_SIZE;
	}

	return h;
}

void env_insert_var(scheme_env* env, sexpr *s) {
	printf("Hash for %s is: %d\n", s->sym, bt_hash(s->sym));
}

sexpr* env_fetch_var(char* key) {
	sexpr *result = sexpr_err("Not implemented yet.");
}

void env_free(scheme_env *env) {
	/* ofc I need to free all the items in all the buckets before I really free it*/

	free(env);
}