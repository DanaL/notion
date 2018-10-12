#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "environment.h"

bucket* bucket_new(char *name, sexpr* e) {
	bucket *b = malloc(sizeof(bucket));
	b->name = malloc(strlen(name) + 1);
	b->val = e;
	b->next = NULL;

	strcpy(b->name, name);

	return b;
}

void bucket_free(bucket* b) {
	free(b->name);

	/* still gotta loop over the chain and free them too */
	free(b);
}

scheme_env* env_new(void) {
	scheme_env *e = malloc(sizeof(scheme_env));
	
	/* I bet there's a fancy C trick to do this better/fast */
	for (int j = 0; j < TABLE_SIZE; j++)
		e->buckets[j] = NULL;

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

void env_insert_var(scheme_env* env, char *name, sexpr *s) {
	int h = bt_hash(name);
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
	int h = bt_hash(key);

	if (env->buckets[h] == NULL)
		return sexpr_err("Idenfifier not found.");

	sexpr *result = env->buckets[h]->val;

	return result;
}

void env_free(scheme_env *env) {
	/* ofc I need to free all the items in all the buckets before I really free it*/

	free(env);
}