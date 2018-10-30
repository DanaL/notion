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

void scope_free(scope *sc) {
	for (unsigned int j = 0; j < sc->size; j++) {
		if (sc->sym_table[j])
			sym_free(sc->sym_table[j]);
	}

	free(sc->sym_table);
	free(sc);
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

void scope_insert_var(scope* sc, char *name, sexpr *exp) {
	unsigned int h = bt_hash(sc->size, name);
	sym *s = sym_new(name, exp);

	if (sc->sym_table[h] == NULL) {
		/* Easy case! Just create the new entry and stick it there */
		sc->sym_table[h] = s;
	}
	else {
		/* A rebinding of the same name in the same scope should free up the
			previous binding */
		sym *existing = sc->sym_table[h];
		while (existing) {
			if (strcmp(existing->name, name) == 0) {
				existing->val = s->val;
				return;
			}

			existing = existing->next;
		}

		s->next = sc->sym_table[h];
		sc->sym_table[h] = s;
	}
}

void scope_insert_global_var(scope *sc, char *name, sexpr *exp) {
	while (sc->parent)
		sc = sc->parent;
	scope_insert_var(sc, name, exp);
}

sexpr* scope_fetch_var(vm_heap *vm, scope *sc, char* key) {
	int h = bt_hash(sc->size, key);

	if (!sc->sym_table[h]) {
		char msg[256];
		snprintf(msg, sizeof msg, "%s%s", "Unbound symbol: ", key);
		sexpr *r = CHECK_PARENT_SCOPE(vm, sc, key, msg);
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
		return CHECK_PARENT_SCOPE(vm, sc, key, msg);
	}

	b->val->global_scope = sc->parent ? 0 : 1;

	return b->val;
}

void env_dump(vm_heap *vm, scope* env) {
	sym *b;

	puts("Binding for current scope:");
	for (unsigned int j = 0; j < env->size; j++) {
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
	printf("Heap count: %lu\n", vm->count);
}

vm_heap* vm_new(void) {
	vm_heap *vm = malloc(sizeof(vm_heap));
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

void vm_free(vm_heap *vm) {
	sexpr *node;

	while (vm->heap) {
		node = vm->heap;
		vm->heap = vm->heap->neighbour;
		sexpr_free(node);
	}
}

void mark_chain(vm_heap* vm, sexpr *chain) {
	/* Bailing out if it has been marked avoids cycles in the graph of
		connection objects */
	if (chain->gen == vm->gc_generation)
		return;

	if (chain->type == LVAL_LIST && chain->count > 0) {
		for (int j = 0; j < chain->count; j++)
			mark_chain(vm, chain->children[j]);
	}
	else if (chain->type == LVAL_FUN && !chain->builtin) {
		mark_chain(vm, chain->params);
		mark_chain(vm, chain->body);
	}

	chain->gen = vm->gc_generation;
}

/* The garbage collector is a simple mark-and-sweep algorithm. Loop through
	the symbol table and mark off any s-expressions that are still in use,
	then loop over the VM's heap linked list and prune any items whose
	generation count is less than current generation.

	Note -- built-in functions are stored in the symbol table but they aren't
	in the heap so they won't be deleted by the garbage collector */
void gc_run(vm_heap* vm, scope* env) {
	vm->gc_generation++;

	/* Need to pass over the entire symbol table and mark which objects
		are still referenced. Don't bother marking built-ins because we are
		never going to recycle them. */
	for (unsigned int j = 0; j < env->size; j++) {
		sym *s = env->sym_table[j];

		while (s) {
			mark_chain(vm, s->val);
			s = s->next;
		}
	}

	sexpr *dead, *prev = NULL;
	sexpr *h = vm->heap;
	int swept = 0;
	while (h) {
		if (h->gen < vm->gc_generation) {
			dead = h;

			if (!prev)
				vm->heap = h->neighbour;
			else
				prev->neighbour = h->neighbour;
			h = h->neighbour;

			sexpr_free(dead);
			--vm->count;
			++swept;
		}
		else {
			prev = h;
			h = h->neighbour;
		}
	}

	printf("%d s-exprs deleted.\n", swept);
}
