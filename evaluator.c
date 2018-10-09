#include <stdio.h>
#include <math.h>
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
		case LVAL_LIST:
			printf("list");
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

int is_zero(lval *num) {
	if (num->num_type == NUM_TYPE_INT)
		return num->n.i_num == 0;
	else
		return (fabs(0 - num->n.d_num) < 0.00000001);
}

lval* builtin_op(lval **nodes, int count) {
	if (nodes[0]->type != LVAL_SYM)
		return lval_err("Expected symbol/built-in op!");

	char *op = nodes[0]->sym;
	lval *result = NULL;

	/* I am not going to be fussy about the difference between integers
		and real numbers. If I am evaluating: + 1 2 14.0, then I'll just 
		convert the result type to float when I hit the real number */
	if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) {
		/* Unary subtraction */
		if (strcmp(op, "-") == 0 && count == 2) {
			if (nodes[1]->type != LVAL_NUM)
				return lval_err("Expected number!");
			result = lval_num_s("0");
			lval_num_sub(result, nodes[1]);
			return result;
		}

		for (int j = 1; j < count; j++) {
			/* The first number value after the operator is result's starting value */
			if (j == 1) {
				result = lval_num(nodes[j]);
				continue;
			}
		
			if (nodes[j]->type == LVAL_NUM) {
				if (strcmp(op, "+") == 0)
					lval_num_add(result, nodes[j]);
				else if (strcmp(op, "-") == 0)
					lval_num_sub(result, nodes[j]);
				else if (strcmp(op, "*") == 0)
					lval_num_mul(result, nodes[j]);
				else if (strcmp(op, "/") == 0) {
					/* Let's make sure we're not trying to divide by zero */
					if (is_zero(nodes[j])) {
						lval_free(result);
						return lval_err("Division by zero!");
					}
					lval_num_div(result, nodes[j]);
				}
				else if (strcmp(op, "%") == 0) {
					if (nodes[j]->num_type != NUM_TYPE_INT || nodes[1]->num_type != NUM_TYPE_INT) {
						lval_free(result);
						return lval_err("Can only calculate the remainder for integers.");
					}
					else if (is_zero(nodes[j])) {
						lval_free(result);
						return lval_err("Division by zero!");
					}
					result->n.i_num %= nodes[j]->n.i_num;
				}
			}
			else {
				lval_free(result);
				return lval_err("Expected number!");
			}
		}		
	}

	if (strcmp(op, "min") == 0) {
		for (int j = 1; j < count; j++) {
			if (j == 1) {
				result = lval_num(nodes[j]);
				continue;
			}

			lval *n = nodes[j];
			if (n->type == LVAL_NUM) {
				if (result->num_type == NUM_TYPE_INT) {
					if (n->num_type == NUM_TYPE_INT && n->n.i_num < result->n.i_num) 
						result->n.i_num = n->n.i_num;
					else if (n->num_type == NUM_TYPE_DEC && n->n.d_num < result->n.i_num) {
						/* We need to switch the number type of result to be a real number in this case */
						lval_convert_num_type(result);
						result->n.d_num = n->n.d_num;
					}
				}
				else {
					if (n->num_type == NUM_TYPE_INT && n->n.i_num < result->n.d_num)
						result->n.d_num = n->n.i_num;
					else if (n->num_type == NUM_TYPE_DEC && n->n.d_num < result->n.d_num) 
						result->n.d_num = n->n.d_num;
				}
			}
			else {
				lval_free(result);
				return lval_err("Expected number!");
			}
		}
	}

	if (strcmp(op, "max") == 0) {
		for (int j = 1; j < count; j++) {
			if (j == 1) {
				result = lval_num(nodes[j]);
				continue;
			}

			lval *n = nodes[j];
			if (n->type == LVAL_NUM) {
				if (result->num_type == NUM_TYPE_INT) {
					if (n->num_type == NUM_TYPE_INT && n->n.i_num > result->n.i_num) 
						result->n.i_num = n->n.i_num;
					else if (n->num_type == NUM_TYPE_DEC && n->n.d_num > result->n.i_num) {
						/* We need to switch the number type of result to be a real number in this case */
						lval_convert_num_type(result);
						result->n.d_num = n->n.d_num;
					}
				}
				else {
					if (n->num_type == NUM_TYPE_INT && n->n.i_num > result->n.d_num)
						result->n.d_num = n->n.i_num;
					else if (n->num_type == NUM_TYPE_DEC && n->n.d_num > result->n.d_num) 
						result->n.d_num = n->n.d_num;
				}
			}
			else {
				lval_free(result);
				return lval_err("Expected number!");
			}
		}
	}

	if (strcmp(op, "list") == 0) {
		result = lval_list();

		for (int j = 1; j < count; j++) {
			lval *cp = NULL;
			if (nodes[j]->type == LVAL_NUM || nodes[j]->type == LVAL_SYM) {
				cp = lval_copy_atom(nodes[j]);
				if (cp->type == LVAL_ERR) {
					lval_free(result);
					return cp;
				}
			}
			else if (nodes[j]->type == LVAL_LIST) {
				cp = lval_list();
				lval_copy_list(cp, nodes[j]);		
			}

			lval_append(result, cp);
		}
	}

	if (strcmp(op, "car") == 0) {
		if (count != 2)
			return lval_err("car expects only one parameter");

		if (nodes[1]->type != LVAL_LIST || nodes[1]->count == 0)
			return lval_err("car is defined only for non-empty lists.");

		lval *l = nodes[1];
		if (l->children[0]->type == LVAL_LIST) {
			result = lval_list();
			lval_copy_list(result, l->children[0]);
		}
		else 
			result = lval_copy_atom(l->children[0]);
	}

	if (strcmp(op, "cdr") == 0) {
		if (count != 2)
			return lval_err("cdr expects only one parameter");

		if (nodes[1]->type != LVAL_LIST || nodes[1]->count == 0)
			return lval_err("cdr is defined only for non-empty lists.");

		result = lval_list();
		lval *l = nodes[1];
		for (int j = 1; j < l->count; j++) {
			if (l->children[j]->type == LVAL_LIST) {
				lval *lst = lval_list();
				lval_copy_list(lst, l->children[j]);
				lval_append(result, lst);
			}
			else {
				lval *a = lval_copy_atom(l->children[j]);
				lval_append(result, a);
			}
		}
	}

	return result;
}

lval* lval_eval(lval *v) {
	lval *result = NULL;

	switch (v->type) {
		case LVAL_SEXPR:
			/* Evaluate all the child expressions first, which makes the code in 
				builtin_op() simpler */
			for (int j = 0; j < v->count; j++) {
				if (v->children[j]->type == LVAL_SEXPR) {
					lval *result = lval_eval(v->children[j]);
					if (result->type == LVAL_ERR)
						return result;
					lval_free(v->children[j]);
					v->children[j] = result;
				}
			}

			result = builtin_op(v->children, v->count);
			return result;
			break;
	}

	return lval_err("Something hasn't been implemented yet");
}
