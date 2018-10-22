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

scope* scope_new(unsigned int size) {
	scope *e = malloc(sizeof(scope));
	e->buckets = calloc(size, sizeof(bucket*));
	e->size = size;
	e->parent = NULL;

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

void scope_insert_var(scope* sc, char *name, sexpr *s) {
	unsigned int h = bt_hash(sc->size, name);
	bucket *b = bucket_new(name, s);

	if (sc->buckets[h] == NULL) {
		/* Easy case! Just create the new bucket and stick it there */
		sc->buckets[h] = b;
	}
	else {
		/* Collision! I am going to make the assumption that a variable added will
			also likely be called soon, so I will put it at the head of th chain */

		/* Note that I am curently in no way handling what to do when a variable with the same name is added */
		/* Or scope...I bet scope is going to be a huge pain... */
		b->next = sc->buckets[h];
		sc->buckets[h] = b;
	}
}

sexpr* scope_fetch_var(scope *sc, char* key) {
	int h = bt_hash(sc->size, key);

	if (!sc->buckets[h]) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		sexpr *r = CHECK_PARENT_SCOPE(sc, key, msg);
		return r;
	}
	bucket *b = sc->buckets[h];

	/* Assuming a variable called is probably going to be
			called again soon, it might be worth moving it to
			the top of the chain. But that's more complicated
			code */
	while (b && strcmp(b->name, key) != 0)
		b = b->next;

	if (!b) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		return CHECK_PARENT_SCOPE(sc, key, msg);
	}

	return sexpr_copy(b->val);
}

void scope_free(scope *sc) {
	for (int j = 0; j < sc->size; j++) {
		if (sc->buckets[j])
			bucket_free(sc->buckets[j]);
	}

	free(sc);
}

void env_dump(scope* env) {
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
