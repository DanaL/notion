#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "evaluator.h"

void print_lval_type(enum lval_type t) {
	switch (t) {
		case LVAL_NUM:
			printf("number");
			break;
		case LVAL_SYM:
			printf("symbol");
			break;
		case LVAL_SEXPR:
			printf("s-expr");
			break;
		case LVAL_ERR:
			printf("error");
			break;
	}
}

void lval_num_add(lval *result, lval *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) {
		long x = result->n.i_num;
		result->n.d_num = x;
		result->num_type = NUM_TYPE_DEC;
	}

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num += v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num += v->n.i_num; 
	}
}

lval* builtin_op(lval **nodes, int count) {
	if (nodes[0]->type != LVAL_SYM)
		return lval_err("Expected symbol/built-in op!");

	char *op = nodes[0]->sym;
	lval *result = NULL;

	if (strcmp(op, "+") == 0) {
		result = lval_num("0");
		for (int j = 1; j < count; j++) {
			/* I am not going to be fussy about the difference between integers
				and real numbers. If I am evaluating: + 1 2 14.0, then I'll just 
				convert the result type to float when I hit the real number */
			if (nodes[j]->type == LVAL_NUM) {
				lval_num_add(result, nodes[j]);
				//if (nodes[j]->num_type == NUM_TYPE_DEC)
				//	result->n.
			}
			else if (nodes[j]->type == LVAL_SEXPR) {
				/* Complicated case -- gotta recurse and calc the nested expression */
			}
			else {
				lval_free(result);
				return lval_err("Expected number!");
			}
		}
	}

	return result;
}

lval* lval_eval(lval *v) {
	lval *result = NULL;

	switch (v->type) {
		case LVAL_SEXPR:
			result = builtin_op(v->children, v->count);
			return result;
			break;
	}

	return lval_err("Something hasn't been implemented yet");
}
