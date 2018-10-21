#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "sexpr.h"
#include "environment.h"
#include "util.h"

bucket* bucket_new(char *name, sexpr* e) {
	bucket *b = malloc(sizeof(bucket));
	b->val = e;
	b->next = NULL;

	b->name = n_strcpy(b->name, name);

	return b;
}

void bucket_free(bucket* b) {
	free(b->name);

	bucket *n, *p = b->next;
	while (p) {
		free(p->name);
		sexpr_free(p->val);
		n = p->next;
		free(p);
		p = n;
	}

	sexpr_free(b->val);
	free(b);
}

scheme_env* env_new(unsigned int size) {
	scheme_env *e = malloc(sizeof(scheme_env));
	e->buckets = malloc(size * sizeof(bucket*));
	e->size = size;
	e->parent = NULL;

	/* I bet there's a fancy C trick to do this better/fast */
	for (int j = 0; j < size; j++)
		e->buckets[j] = NULL;

	return e;
}

int bt_hash(unsigned int size, char *s) {
	long h = 0;
	int len = strlen(s);

	for (int j = 0; j < len; j++) {
		h += (long)pow(163, len - (j+1)) * s[j];
		h %= size;
	}

	return h;
}

void env_insert_var(scheme_env* env, char *name, sexpr *s) {
	unsigned int h = bt_hash(env->size, name);
	bucket *b = bucket_new(name, s);

	if (env->buckets[h] == NULL) {
		/* Easy case! Just create the new bucket and stick it there */
		env->buckets[h] = b;
	}
	else {
		/* Collision! I am going to make the assumption that a variable added will
			also likely be called soon, so I will put it at the head of th chain */

		/* Note that I am curently in no way handling what to do when a variable with the same name is added */
		/* Or scope...I bet scope is going to be a huge pain... */
		b->next = env->buckets[h];
		env->buckets[h] = b;
	}
}

sexpr* env_fetch_var(scheme_env *env, char* key) {
	int h = bt_hash(env->size, key);

	if (!env->buckets[h]) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		sexpr *r = CHECK_PARENT_SCOPE(env, key, msg);
		return r;
	}
	bucket *b = env->buckets[h];

	/* Assuming a variable called is probably going to be
			called again soon, it might be worth moving it to
			the top of the chain. But that's more complicated
			code */
	while (b && strcmp(b->name, key) != 0)
		b = b->next;

	if (!b) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		return CHECK_PARENT_SCOPE(env, key, msg);
	}

	return sexpr_copy(b->val);
}

void env_free(scheme_env *env) {
	for (int j = 0; j < env->size; j++) {
		if (env->buckets[j])
			bucket_free(env->buckets[j]);
	}

	free(env);
}

void env_dump(scheme_env* env) {
	bucket *b;

	puts("Binding for current scope:");
	for (int j = 0; j < env->size; j++) {
		b = env->buckets[j];
		if (b) {
			printf("  %s ", b->name);
			print_sexpr_type(b->val);
			putchar(' ');
			sexpr_pprint(b->val);
			putchar('\n');
		}
	}
}
