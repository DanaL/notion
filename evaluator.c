#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "evaluator.h"

void print_sexpr_type(sexpr *v) {
	switch (v->type) {
		case LVAL_NUM:
			printf("number");
			break;
		case LVAL_SYM:
			printf("symbol (%s)", v->sym);
			break;
		case LVAL_ERR:
			printf("error");
			break;
		case LVAL_LIST:
			printf("list");
			break;
	}
}

void sexpr_convert_num_type(sexpr *v) {
	long x = v->n.i_num;
	v->n.d_num = x;
	v->num_type = NUM_TYPE_DEC;
}

void sexpr_num_add(sexpr *result, sexpr *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		sexpr_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num += v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num += v->n.i_num; 
	}
}

void sexpr_num_sub(sexpr *result, sexpr *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		sexpr_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num -= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num -= v->n.i_num; 
	}
}

void sexpr_num_mul(sexpr *result, sexpr *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		sexpr_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num *= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num *= v->n.i_num; 
	}
}

void sexpr_num_div(sexpr *result, sexpr *v) {
	if (result->num_type == NUM_TYPE_INT && v->num_type == NUM_TYPE_DEC) 
		sexpr_convert_num_type(result);

	if (result->num_type == NUM_TYPE_DEC) {
		result->n.d_num /= v->num_type == NUM_TYPE_INT ? v->n.i_num : v->n.d_num;
	}
	else {
		result->n.i_num /= v->n.i_num; 
	}
}

int is_zero(sexpr *num) {
	if (num->num_type == NUM_TYPE_INT)
		return num->n.i_num == 0;
	else
		return (fabs(0 - num->n.d_num) < 0.00000001);
}

sexpr* builtin_math_op(sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	/* Unary subtraction */
	if (strcmp(op, "-") == 0 && count == 2) {
		if (nodes[1]->type != LVAL_NUM)
			return sexpr_err("Expected number!");
		result = sexpr_num_s("0");
		sexpr_num_sub(result, nodes[1]);
		return result;
	}

	for (int j = 1; j < count; j++) {		
		/* The first number value after the operator is result's starting value */
		if (j == 1) {
			result = sexpr_num(nodes[j]);			
			continue;
		}
		
		if (nodes[j]->type == LVAL_NUM) {
			if (strcmp(op, "+") == 0)
				sexpr_num_add(result, nodes[j]);
			else if (strcmp(op, "-") == 0)
				sexpr_num_sub(result, nodes[j]);
			else if (strcmp(op, "*") == 0)
				sexpr_num_mul(result, nodes[j]);
			else if (strcmp(op, "/") == 0) {
				/* Let's make sure we're not trying to divide by zero */
				if (is_zero(nodes[j])) {
					sexpr_free(result);
					return sexpr_err("Division by zero!");
				}
				sexpr_num_div(result, nodes[j]);
			}
			else if (strcmp(op, "%") == 0) {
				if (nodes[j]->num_type != NUM_TYPE_INT || nodes[1]->num_type != NUM_TYPE_INT) {
					sexpr_free(result);
					return sexpr_err("Can only calculate the remainder for integers.");
				}
				else if (is_zero(nodes[j])) {
					sexpr_free(result);
					return sexpr_err("Division by zero!");
				}
				result->n.i_num %= nodes[j]->n.i_num;
			}
		}
		else {
			sexpr_free(result);
			return sexpr_err("Expected number!");
		}
	}

	return result;
}

sexpr* builtin_min_op(sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	for (int j = 1; j < count; j++) {
		if (j == 1) {
			result = sexpr_num(nodes[j]);
			continue;
		}

		sexpr *n = nodes[j];
		if (n->type == LVAL_NUM) {
			if (result->num_type == NUM_TYPE_INT) {
				if (n->num_type == NUM_TYPE_INT && n->n.i_num < result->n.i_num) 
					result->n.i_num = n->n.i_num;
				else if (n->num_type == NUM_TYPE_DEC && n->n.d_num < result->n.i_num) {
					/* We need to switch the number type of result to be a real number in this case */
					sexpr_convert_num_type(result);
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
			sexpr_free(result);
			return sexpr_err("Expected number!");
		}
	}

	return result;
}

sexpr* builtin_max_op(sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	for (int j = 1; j < count; j++) {
		if (j == 1) {
			result = sexpr_num(nodes[j]);
			continue;
		}

		sexpr *n = nodes[j];
		if (n->type == LVAL_NUM) {
			if (result->num_type == NUM_TYPE_INT) {
				if (n->num_type == NUM_TYPE_INT && n->n.i_num > result->n.i_num) 
					result->n.i_num = n->n.i_num;
				else if (n->num_type == NUM_TYPE_DEC && n->n.d_num > result->n.i_num) {
					/* We need to switch the number type of result to be a real number in this case */
					sexpr_convert_num_type(result);
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
			sexpr_free(result);
			return sexpr_err("Expected number!");
		}
	}

	return result;
}

sexpr* builtin_op(sexpr **nodes, int count) {
	if (nodes[0]->type != LVAL_SYM)
		return sexpr_err("Expected symbol/built-in op!");

	char *op = nodes[0]->sym;
	sexpr *result = NULL;

	/* I am not going to be fussy about the difference between integers
		and real numbers. If I am evaluating: + 1 2 14.0, then I'll just 
		convert the result type to float when I hit the real number */
	if (strstr("+-*/%", op)) {
		result = builtin_math_op(nodes, count, op);
	}

	if (strcmp(op, "min") == 0) {
		result = builtin_min_op(nodes, count, op);
	}

	if (strcmp(op, "max") == 0) {
		result = builtin_max_op(nodes, count, op);
	}

	if (strcmp(op, "list") == 0) {
		result = sexpr_list();

		for (int j = 1; j < count; j++) {
			sexpr *cp = sexpr_copy(nodes[j]);			
			sexpr_append(result, cp);
		}
	}

	if (strcmp(op, "car") == 0) {
		if (count != 2)
			return sexpr_err("car expects only one argument");

		if (nodes[1]->type != LVAL_LIST || nodes[1]->count == 0)
			return sexpr_err("car is defined only for non-empty lists.");

		sexpr *l = nodes[1];
		result = sexpr_copy(l->children[0]);
	}

	if (strcmp(op, "cdr") == 0) {
		if (count != 2)
			return sexpr_err("cdr expects only one argument");

		if (nodes[1]->type != LVAL_LIST || nodes[1]->count == 0)
			return sexpr_err("cdr is defined only for non-empty lists.");

		result = sexpr_list();
		sexpr *l = nodes[1];
		for (int j = 1; j < l->count; j++) {
			sexpr_list_insert(result, l->children[j]);
		}
	}

	if (strcmp(op, "cons") == 0) {
		if (count != 3)
			return sexpr_err("cons expects two aruments");

		if (nodes[2]->type != LVAL_LIST)
			return sexpr_err("The second argument of cons must be a list.");

		result = sexpr_list();
		sexpr_list_insert(result, nodes[1]);

		for (int j = 0; j < nodes[2]->count; j++) {
			sexpr_list_insert(result, nodes[2]->children[j]);
		}
	}

	return result;
}

int is_built_in(sexpr *e) {	
	if (e->type != LVAL_SYM)
		return 0;

	if (strstr("+-*/%", e->sym))
		return 1;

	int result = 0;

	if (strcmp(e->sym, "max") == 0)
		result = 1;
	else if (strcmp(e->sym, "min") == 0)
		result = 1;
	else if (strcmp(e->sym, "list") == 0)
		result = 1;
	else if (strcmp(e->sym, "car") == 0)
		result = 1;
	else if (strcmp(e->sym, "cdr") == 0)
		result = 1;
	else if (strcmp(e->sym, "cons") == 0)
		result = 1;
	else if (strcmp(e->sym, "quote") == 0)
		result = 1;
	else if (strcmp(e->sym, "'") == 0)
		result = 1;

	return result;
}

int is_quote_form(sexpr *e) {
	if (e->type != LVAL_SYM)
		return 0;

	if (strcmp(e->sym, "'") == 0 || strcmp(e->sym, "quote") == 0)
		return 1;
	
	return 0;	
}

sexpr* eval(sexpr *v) {
	sexpr *result = NULL;

	switch (v->type) {
		case LVAL_LIST:			
			/* When I have user defined functions, I'll have to check if the
				op is one of them */
			if (v->count == 0 || !is_built_in(v->children[0]))
				return sexpr_err("Expected operator or function");
			
			/* quote is a special form -- we simply return its first paramter without evaluating it.
				Note that I am not certain I really understand quote, so I'm mostly going by what's in
				the Little Schemer and mucking around with other Scheme REPLs. 

				For instance (quote 1 2 3) returns 1 but I'm not sure if that's valid input from a proper
				definition of Scheme or not.
				*/
			if (is_quote_form(v->children[0])) {
				return sexpr_copy(v->children[1]);
			}

			/* Evaluate all the child expressions first, which makes the code in 
				builtin_op() simpler */
			for (int j = 1; j < v->count; j++) {
				if (v->children[j]->type == LVAL_LIST) {
					sexpr *result = eval(v->children[j]);
					if (result->type == LVAL_ERR)
						return result;
					sexpr_free(v->children[j]);
					v->children[j] = result;
				}
			}

			result = builtin_op(v->children, v->count);
			return result;			
		case LVAL_NUM:
		case LVAL_SYM:
			return sexpr_copy(v);
	}

	return sexpr_err("Something hasn't been implemented yet");
}
