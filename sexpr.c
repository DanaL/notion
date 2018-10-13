#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexpr.h"

sexpr* sexpr_num(sexpr* j) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;
	v->num_type = j->num_type;

	if (j->num_type == NUM_TYPE_INT) {
		v->n.i_num = j->n.i_num;
	}
	else
		v->n.d_num = j->n.d_num;

	return v;
}

sexpr* sexpr_num_s(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;

	if (strchr(s, '.')) {
		float f = strtof(s, NULL);
		v->num_type = NUM_TYPE_DEC;
		v->n.d_num = f;
	}
	else {
		long i = strtol(s, NULL, 10);
		v->num_type = NUM_TYPE_INT;
		v->n.i_num = i;
	}

	return v;
}

sexpr* sexpr_err(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(s) + 1);
	strcpy(v->err, s);

	return v;
}

sexpr* sexpr_sym(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);

	return v;
}

sexpr* sexpr_bool(int v) {
		sexpr *b = malloc(sizeof(sexpr));
		b->type = LVAL_BOOL;
		b->bool = v;

		return b;
}

sexpr* sexpr_list(void) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_LIST;
	v->count = 0;
	v->children = NULL;

	return v;
}

sexpr* sexpr_null(void) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NULL;
	v->count = 0;
	v->children = NULL;

	return v;
}

void sexpr_free(sexpr *v) {
	switch (v->type) {
		case LVAL_NULL:
		case LVAL_NUM:
		case LVAL_BOOL:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_LIST:
			for (int j = 0; j < v->count; j++)
				sexpr_free(v->children[j]);
			break;
	}

	free(v);
}

sexpr* sexpr_copy_atom(sexpr* src) {
	if (src->type == LVAL_NUM)
		return sexpr_num(src);

	if (src->type == LVAL_SYM)
		return sexpr_sym(src->sym);

	if (src->type == LVAL_BOOL)
		return sexpr_bool(src->bool);

	if (src->type == LVAL_NULL)
		return sexpr_null();

	return sexpr_err("Can only copy atoms.");
}

sexpr* sexpr_copy_list(sexpr* src) {
	sexpr *dst = sexpr_list();

	for (int j = 0; j < src->count; j++) {
		if (IS_ATOM(src->children[j])) {
			sexpr *cp = sexpr_copy_atom(src->children[j]);
			sexpr_append(dst, cp);
		}
		else if (src->children[j]->type == LVAL_LIST) {
			sexpr_append(dst, sexpr_copy_list(src->children[j]));
		}
	}

	return dst;
}

sexpr* sexpr_copy(sexpr* src) {
	if (IS_ATOM(src))
		return sexpr_copy_atom(src);

	return sexpr_copy_list(src);
}

void sexpr_append(sexpr *v, sexpr *next) {
	v->count++;
	v->children = realloc(v->children, sizeof(sexpr*) * v->count);
	v->children[v->count - 1] = next;
}

void sexpr_list_insert(sexpr *dst, sexpr *item) {
	sexpr* cp = sexpr_copy(item);
	sexpr_append(dst, cp);
}

void print_padding(int depth) {
	for (int j = 0; j < depth * 4; j++) putchar(' ');
}

void sexpr_pprint(sexpr *v, int depth) {
	switch (v->type) {
		case LVAL_ERR:
			printf("Error: %s\n", v->err);
			break;
		case LVAL_LIST:
			if (depth > 0)
				putchar('\n');
			print_padding(depth);
			putchar('(');

			for (int j = 0; j < v->count; j++) {
				sexpr_pprint(v->children[j], depth + 1);
				if (j < v->count - 1)
					putchar(' ');
			}
			puts(")");
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_NUM:
			if (v->num_type == NUM_TYPE_INT)
				printf("%li", v->n.i_num);
			else
				printf("%f", v->n.d_num);
			break;
		case LVAL_BOOL:
			printf("%s", v->bool ? "true" : "false");
			break;
		case LVAL_NULL:
			/* Don't need to do anything */
			break;
	}
}
