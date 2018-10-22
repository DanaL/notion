#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "sexpr.h"
#include "environment.h"
#include "util.h"

sym* sym_new(char *name, sexpr* e) {
	sym *b = malloc(sizeof(sym));
	b->val = e;
	b->next = NULL;

	b->name = n_strcpy(b->name, name);

	return b;
}

void sym_free(sym* b) {
	free(b->name);

	/* I don't think I'll want to do this once garbage collection is a thing.
		All the sym table entries should be pure references to things on the
		heap. */
	sym *n, *p = b->next;
	while (p) {
		free(p->name);
		n = p->next;
		free(p);
		p = n;
	}

	free(b);
}

scope* scope_new(unsigned int size) {
	scope *e = malloc(sizeof(scope));
	e->sym_table = calloc(size, sizeof(sym*));
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

void scope_insert_var(vm_heap *vm, scope* sc, char *name, sexpr *s) {
	unsigned int h = bt_hash(sc->size, name);
	sym *b = sym_new(name, s);
	vm_add(vm, s);

	if (sc->sym_table[h] == NULL) {
		/* Easy case! Just create the new entry and stick it there */
		sc->sym_table[h] = b;
	}
	else {
		/* Collision! I am going to make the assumption that a variable added will
			also likely be called soon, so I will put it at the head of th chain */

		/* Note that I am curently in no way handling what to do when a variable with the same name is added */
		/* Or scope...I bet scope is going to be a huge pain... */
		b->next = sc->sym_table[h];
		sc->sym_table[h] = b;
	}
}

sexpr* scope_fetch_var(scope *sc, char* key) {
	int h = bt_hash(sc->size, key);

	if (!sc->sym_table[h]) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		sexpr *r = CHECK_PARENT_SCOPE(sc, key, msg);
		return r;
	}
	sym *b = sc->sym_table[h];

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
		if (sc->sym_table[j])
			sym_free(sc->sym_table[j]);
	}

	free(sc);
}

void env_dump(vm_heap *vm, scope* env) {
	sym *b;

	puts("Binding for current scope:");
	for (int j = 0; j < env->size; j++) {
		b = env->sym_table[j];
		if (b) {
			printf("  %s ", b->name);
			print_sexpr_type(b->val);
			putchar(' ');
			sexpr_pprint(b->val);
			putchar('\n');
		}
	}

	printf("\nHeap items:\n");
	printf("Heap count: %ld\n", vm->count);
}

vm_heap* vm_new(void) {
	vm_heap *vm = malloc(sizeof(vm));
	vm->count = 0;
	vm->gc_generation = 0;
	vm->heap = NULL;

	return vm;
}

void vm_add(vm_heap* vm, sexpr* expr) {
	expr->neighbour = vm->heap;
	vm->heap = expr;
	vm->count++;
}
