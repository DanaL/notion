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

void lval_convert_num_type(lval *v) {
	long x = v->n.i_num;
	v->n.d_num = x;
	v->num_type = NUM_TYPE_DEC;
}

void lval_num_add(lval *result, lval *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		lval_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num += v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num += v->n.i_num; 
	}
}

void lval_num_sub(lval *result, lval *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		lval_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num -= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num -= v->n.i_num; 
	}
}

void lval_num_mul(lval *result, lval *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		lval_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num *= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num *= v->n.i_num; 
	}
}

void lval_num_div(lval *result, lval *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		lval_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num /= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num /= v->n.i_num; 
	}
}

void lval_math_op(char *op, lval *result, lval *v) {
	if (strcmp(op, "+") == 0)
		lval_num_add(result, v);
	else if (strcmp(op, "-") == 0)
		lval_num_sub(result, v);
	else if (strcmp(op, "*") == 0)
		lval_num_mul(result, v);
	else if (strcmp(op, "/") == 0)
		lval_num_div(result, v);
}

lval* builtin_op(lval **nodes, int count) {
	if (nodes[0]->type != LVAL_SYM)
		return lval_err("Expected symbol/built-in op!");

	char *op = nodes[0]->sym;
	lval *result = NULL;

	/* I am not going to be fussy about the difference between integers
		and real numbers. If I am evaluating: + 1 2 14.0, then I'll just 
		convert the result type to float when I hit the real number */
	if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
		/* The first number value after the operator is result's starting value */
		for (int j = 1; j < count; j++) {
			if (nodes[j]->type == LVAL_NUM) {
				if (j == 1) 
					result = lval_num(nodes[j]);
				else 
					lval_math_op(op, result, nodes[j]);
			}
			else if (nodes[j]->type == LVAL_SEXPR) {
				/* Complicated case -- gotta recurse and calc the nested expression */
				lval *subexp = lval_eval(nodes[j]);
				if (subexp->type == LVAL_ERR) {
					lval_free(result);
					return subexp;
				}
				else if (subexp->type != LVAL_NUM) {
					lval_free(result);
					lval_free(subexp);
					return lval_err("Expected number!");
				}

				if (j == 1) 
					result = subexp;
				else {
					lval_math_op(op, result, subexp);
					free(subexp);
				}
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
