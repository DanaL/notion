#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sexpr.h"
#include "util.h"

/* Making this a singleton of sorts. A bunch of functions return a null value
	but they don't need to be unique values stored on the heap and re-created
	and garbage collected. Re-use is better than recycling! */
static sexpr *null_expr = NULL;

void print_sexpr_type(sexpr *v) {
	switch (v->type) {
		case LVAL_NUM:
			printf("number");
			break;
		case LVAL_STR:
			printf("string (%s)", v->str);
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

char* sexpr_desc(sexpr *v) {
	char buffer[1024];

	switch (v->type) {
		case LVAL_NUM:
			if (v->num_type == NUM_TYPE_INT)
				snprintf(buffer, sizeof buffer, "%ld", v->i_num);
			else
				snprintf(buffer, sizeof buffer, "%f", v->d_num);
			break;
		case LVAL_STR:
			snprintf(buffer, sizeof buffer, "\"%s\"", v->str);
			break;
		case LVAL_FUN:
		case LVAL_SYM:
			snprintf(buffer, sizeof buffer, "%s", v->sym);
			break;
		case LVAL_ERR:
			snprintf(buffer, sizeof buffer, "Err: %s", v->sym);
			break;
		case LVAL_LIST:
			snprintf(buffer, sizeof buffer, "List");
			break;
		case LVAL_NULL:
			snprintf(buffer, sizeof buffer, "Null");
			break;
		case LVAL_BOOL:
			snprintf(buffer, sizeof buffer, "%s", v->bool ? "#t" : "#f");
			break;
	}

	char *msg = NULL;
	msg = n_strcpy(msg, buffer);

	return msg;
}

/* This is mathematically, philosophically terrible, but also so
	terribly convenient */
sexpr* sexpr_num(vm_heap* vm, enum sexpr_num_type t, double n) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;
	v->num_type = t;
	v->gen = 0;
	v->count = 0;

	if (t == NUM_TYPE_INT) {
		v->i_num = n;
	}
	else
		v->d_num = n;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_num_s(vm_heap* vm, char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;
	v->gen = 0;
	v->count = 0;

	if (strchr(s, '.')) {
		double f = strtod(s, NULL);
		v->num_type = NUM_TYPE_DEC;
		v->d_num = f;
	}
	else {
		long i = strtol(s, NULL, 10);
		v->num_type = NUM_TYPE_INT;
		v->i_num = i;
	}

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_fun_builtin(builtinf fun, char *name) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_FUN;
	v->fun = fun;
	v->sym = n_strcpy(v->sym, name);
	v->builtin = 1;
	v->params = NULL;
	v->body = NULL;
	v->gen = 0;
	v->count = 0;

	return v;
}

sexpr* sexpr_fun_user(vm_heap* vm, sexpr *params, sexpr *body, char *name) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_FUN;
	v->fun = NULL;
	v->sym = n_strcpy(v->sym, name);
	v->builtin = 0;
	v->params = params;
	v->body = body;
	v->gen = 0;
	v->count = 0;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_str(vm_heap* vm, char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_STR;
	v->gen = 0;
	v->count = 0;

	if (s)
		v->str = n_strcpy(v->str, s);
	else
		v-> str = NULL;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_err(vm_heap* vm, char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_ERR;
	v->err = n_strcpy(v->err, s);
	v->gen = 0;
	v->count = 0;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_sym(vm_heap* vm, char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_SYM;
	v->sym = n_strcpy(v->sym, s);
	v->gen = 0;
	v->count = 0;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_bool(vm_heap* vm, int v) {
	sexpr *b = malloc(sizeof(sexpr));
	b->type = LVAL_BOOL;
	b->bool = v;
	b->gen = 0;
	b->count = 0;

	vm_add(vm, b);

	return b;
}

sexpr* sexpr_list(vm_heap* vm) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_LIST;
	v->count = 0;
	v->children = NULL;
	v->gen = 0;

	vm_add(vm, v);

	return v;
}

sexpr* sexpr_null(void) {
	if (!null_expr) {
		null_expr = malloc(sizeof(sexpr));
		null_expr->type = LVAL_NULL;
		null_expr->sym = NULL;
		null_expr->err = NULL;
		null_expr->str = NULL;
		null_expr->count = 0;
		null_expr->children = NULL;
		null_expr->neighbour = NULL;
		null_expr->params = NULL;
		null_expr->body = NULL;
		null_expr->fun = NULL;
		null_expr->gen = 0;
		null_expr->count = 0;
	}

	return null_expr;
}

void sexpr_free(sexpr *v) {
	switch (v->type) {
		case LVAL_LIST:
			free(v->children);
			break;
		case LVAL_NUM:
		case LVAL_BOOL:
		case LVAL_NULL:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_FUN:
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_STR:
			if (v->str)
				free(v->str);
			break;
	}

	free(v);
}

sexpr* sexpr_copy_atom(vm_heap* vm, sexpr* src) {
	if (src->type == LVAL_NUM)
		return sexpr_num(vm, src->num_type, NUM_CONVERT(src));

	if (src->type == LVAL_SYM)
		return sexpr_sym(vm, src->sym);

	if (src->type == LVAL_BOOL)
		return sexpr_bool(vm, src->bool);

	if (src->type == LVAL_NULL)
		return src;

	if (src->type == LVAL_FUN) {
		if (src->builtin)
			return sexpr_fun_builtin(src->fun, src->sym);
		else
			return sexpr_fun_user(vm, sexpr_copy(vm, src->params),
						sexpr_copy(vm, src->body), src->sym);
	}

	if (src->type == LVAL_STR)
		return sexpr_str(vm, src->str);

	return sexpr_err(vm, "Can only copy atoms.");
}

sexpr* sexpr_copy_list(vm_heap* vm, sexpr* src) {
	sexpr *dst = sexpr_list(vm);

	for (int j = 0; j < src->count; j++) {
		if (IS_ATOM(src->children[j])) {
			sexpr *cp = sexpr_copy_atom(vm, src->children[j]);
			sexpr_append(dst, cp);
		}
		else if (src->children[j]->type == LVAL_LIST) {
			sexpr_append(dst, sexpr_copy_list(vm, src->children[j]));
		}
	}

	return dst;
}

sexpr* sexpr_copy(vm_heap* vm, sexpr* src) {
	if (IS_ATOM(src))
		return sexpr_copy_atom(vm, src);

	return sexpr_copy_list(vm, src);
}

void sexpr_append(sexpr *v, sexpr *next) {
	v->count++;
	v->children = realloc(v->children, sizeof(sexpr*) * v->count);
	v->children[v->count - 1] = next;
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
		case LVAL_STR:
			printf("\"%s\"", v->str);
			break;
		case LVAL_NUM:
			if (v->num_type == NUM_TYPE_INT)
				printf("%li", v->i_num);
			else
				printf("%f", v->d_num);
			break;
		case LVAL_BOOL:
			printf("%s", v->bool ? "#t" : "#f");
			break;
		case LVAL_FUN:
			if (v->builtin)
				printf("Built-in function");
			else {
				printf("User-defined function");
				putchar(' ');
				sexpr_pprint(v->params);
				putchar(' ');
				sexpr_pprint(v->body);
			}
		case LVAL_NULL:
			/* Don't need to do anything */
			break;
	}
}
