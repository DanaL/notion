#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexpr.h"
#include "util.h"

void print_sexpr_type(sexpr *v) {
	switch (v->type) {
		case LVAL_NUM:
			printf("number");
			break;
		case LVAL_SYM:
			printf("symbol (%s)", v->sym);
			break;
		case LVAL_FUN:
			printf("func (%s)", v->sym);
			break;
		case LVAL_ERR:
			printf("error");
			break;
		case LVAL_LIST:
			printf("list");
			break;
		case LVAL_NULL:
			printf("null type");
			break;
		case LVAL_BOOL:
			printf("boolean (%s)", v->bool ? "true" : "false");
			break;
	}
}

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

sexpr* sexpr_fun(builtinf fun, char *name) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_FUN;
	v->fun = fun;
	v->sym = n_strcpy(v->sym, name);

	return v;
}

sexpr* sexpr_err(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_ERR;
	v->err = n_strcpy(v->err, s);

	return v;
}

sexpr* sexpr_sym(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_SYM;
	v->sym = n_strcpy(v->sym, s);

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
		case LVAL_FUN:
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

	if (src->type == LVAL_FUN)
		return sexpr_fun(src->fun, src->sym);

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

void sexpr_pprint(sexpr *v) {
	switch (v->type) {
		case LVAL_ERR:
			printf("Error: %s\n", v->err);
			break;
		case LVAL_LIST:
			putchar('(');

			for (int j = 0; j < v->count; j++) {
				sexpr_pprint(v->children[j]);
				if (j < v->count - 1)
					putchar(' ');
			}
			putchar(')');
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
		case LVAL_FUN:
			printf("Function type");
		case LVAL_NULL:
			/* Don't need to do anything */
			break;
	}
}
