#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "environment.h"
#include "sexpr.h"
#include "parser.h"
#include "util.h"

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
		case LVAL_NULL:
			printf("null type");
			break;
		case LVAL_BOOL:
			printf("boolean (%s)", v->bool ? "true" : "false");
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

int sexpr_cmp(sexpr *s1, sexpr *s2) {
	if (s1->type != s2->type)
		return 0;

	switch (s1->type) {
		case LVAL_BOOL:
			if (s1->bool != s2->bool)
				return 0;
			break;
		case LVAL_NUM:
			if (s1->num_type != s2->num_type)
				return 0;
			if (s1->num_type == NUM_TYPE_INT && s1->n.i_num != s2->n.i_num)
				return 0;
			if (s1->num_type == NUM_TYPE_DEC && s1->n.d_num != s2->n.d_num)
				return 0;
			break;
		case LVAL_SYM:
			if (strcmp(s1->sym, s2->sym) != 0)
				return 0;
			break;
		case LVAL_ERR:
			if (strcmp(s1->err, s2->err) != 0)
				return 0;
			break;
		case LVAL_LIST:
			if (s1->count != s2->count)
			 	return 0;

			for (int j = 0; j < s1->count; j++) {
				if (!sexpr_cmp(s1->children[j], s2->children[j]))
					return 0;
			}

			break;
		case LVAL_NULL:
			return 1;
	}

	return 1;
}

int test_sexpr(scheme_env *env, char *s, sexpr *expected) {
	int c = 0;
	char *line = n_strcpy(line, s);
	sexpr *ast = parse(line, &c);
	printf("Checking: %s\n", line);
	sexpr *result = eval(env, ast);

	int r = sexpr_cmp(result, expected);
	sexpr_free(result);
	sexpr_free(ast);
	free(line);

	if (!r)
		printf("** Test %s failed T_T\n", s);

	return r;
}

sexpr* builtin_self_test(scheme_env *env, sexpr **nodes, int count, char *op) {
	scheme_env *test_env = env_new(); /* We'll test in a fresh environment */
	sexpr *null_e = sexpr_null();
	int r = 1;

	sexpr *p = sexpr_num_s("4");
	if (!test_sexpr(test_env, "(/ (car (cdr (cdr(list(car (car (cdr (list 1 (list 2 3) (list (list 4)))))) 4 8)))) 2)", p))
		r = 0;
	sexpr_free(p);

	if (!test_sexpr(test_env, "(define f 'list)", null_e))
		r = 0;
	if (!test_sexpr(test_env, "(define f2 '((eval f) x y))", null_e))
		r = 0;
	if (!test_sexpr(test_env, "(define x 4)", null_e))
		r = 0;
	if (!test_sexpr(test_env, "(define y 5)", null_e))
		r = 0;

	int c = 0;
	p = parse("(list 4 5)", &c);
	sexpr *expected = eval(test_env, p);
	if (!test_sexpr(test_env, "(eval f2)", expected))
		r = 0;
	sexpr_free(p);
	sexpr_free(expected);

	if (!test_sexpr(test_env, "(define a #t)", null_e))
		r = 0;

	p = sexpr_bool(1);
	if (!test_sexpr(test_env, "(eq? a #t)", p))
		r = 0;
	sexpr_free(p);

	p = sexpr_bool(0);
	if (!test_sexpr(test_env, "(eq? a #f)", p))
		r = 0;
	sexpr_free(p);

	sexpr_free(null_e);
	env_free(test_env);

	return sexpr_bool(r);
}

/* This is function where we sort out what something is. If passed a simple
	type (like a number), just return it. If passed a symbol, try to
	resolve it. And if passed a list, evaluate it.

	Return a copy of the result so that we can free() it without worrying
	about clobbering a reference somewhere.
*/
sexpr* resolve_sexp(scheme_env *env, sexpr *a) {
	if (a->type == LVAL_NUM || a->type == LVAL_BOOL)
		return sexpr_copy(a);

	if (a->type == LVAL_SYM || IS_FUNC(a))
		return eval(env, a);

	return sexpr_err("Unknown but definitely bad result of resolve_sexp()");
}

sexpr* builtin_math_op(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	/* Unary subtraction */
	if (strcmp(op, "-") == 0 && count == 2) {
		sexpr *n = resolve_sexp(env, nodes[1]);
		if (n->type != LVAL_NUM)
			result = sexpr_err("Expected number!");
		else {
			sexpr_num_sub(result, nodes[1]);
			result = sexpr_num_s("0");
		}
		sexpr_free(n);

		return result;
	}

	for (int j = 1; j < count; j++) {
		sexpr *n = resolve_sexp(env, nodes[j]);

		if (n->type != LVAL_NUM) {
			if (result) sexpr_free(result);
			sexpr_free(n);
			return sexpr_err("Expected number!");
		}
		/* The first number value after the operator is result's starting value */
		if (j == 1) {
			result = sexpr_num(n);
			continue;
		}

		if (strcmp(op, "+") == 0)
			sexpr_num_add(result, n);
		else if (strcmp(op, "-") == 0)
			sexpr_num_sub(result, n);
		else if (strcmp(op, "*") == 0)
			sexpr_num_mul(result, n);
		else if (strcmp(op, "/") == 0) {
			/* Let's make sure we're not trying to divide by zero */
			if (is_zero(n)) {
				sexpr_free(n);
				sexpr_free(result);
				return sexpr_err("Division by zero!");
			}
			sexpr_num_div(result, n);
		}
		else if (strcmp(op, "%") == 0) {
			if (n->num_type != NUM_TYPE_INT || nodes[1]->num_type != NUM_TYPE_INT) {
				sexpr_free(n);
				sexpr_free(result);
				return sexpr_err("Can only calculate the remainder for integers.");
			}
			else if (is_zero(n)) {
				sexpr_free(n);
				sexpr_free(result);
				return sexpr_err("Division by zero!");
			}
			result->n.i_num %= n->n.i_num;
		}
	}

	return result;
}

sexpr* builtin_min_op(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	for (int j = 1; j < count; j++) {
		sexpr *n = resolve_sexp(env, nodes[j]);
		/* This check can be macro-ized, I think */
		if (n->type != LVAL_NUM) {
			if (result)
				sexpr_free(result);
			sexpr_free(n);
			return sexpr_err("Expected number!");
		}

		if (j == 1) {
			result = sexpr_num(n);
			sexpr_free(n);
			continue;
		}

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

		sexpr_free(n);
	}

	return result;
}

sexpr* builtin_max_op(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	for (int j = 1; j < count; j++) {
		sexpr *n = resolve_sexp(env, nodes[j]);
		if (n->type != LVAL_NUM) {
			if (result)
				sexpr_free(result);
			sexpr_free(n);
			return sexpr_err("Expected number!");
		}

		if (j == 1) {
			result = sexpr_num(n);
			continue;
		}

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

		sexpr_free(n);
	}

	return result;
}

sexpr* builtin_list(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	for (int j = 1; j < count; j++) {
		sexpr *cp = resolve_sexp(env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			sexpr_free(result);
			return cp;
		}

		sexpr_append(result, cp);
	}

	return result;
}

sexpr* builtin_cdr(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("cdr expects only one argument");

	sexpr *l = resolve_sexp(env, nodes[1]);
	if (l->type != LVAL_LIST || l->count == 0) {
		sexpr_free(l);
		return sexpr_err("cdr is defined only for non-empty lists.");
	}

	sexpr *result = sexpr_list();
	for (int j = 1; j < l->count; j++) {
		sexpr_list_insert(result, sexpr_copy(l->children[j]));
	}
	sexpr_free(l);

	return result;
}

sexpr* builtin_car(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("car expects only one argument");

	sexpr *l = resolve_sexp(env, nodes[1]);

	if (l->type != LVAL_LIST || l->count == 0) {
		sexpr_free(l);
		return sexpr_err("car is defined only for non-empty lists.");
	}

	sexpr *result = sexpr_copy(l->children[0]);

	sexpr_free(l);

	return result;
}

sexpr* builtin_cons(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3)
		return sexpr_err("cons expects two aruments");

	sexpr *a2 = resolve_sexp(env, nodes[2]);

	if (a2->type != LVAL_LIST) {
		sexpr_free(a2);
		return sexpr_err("The second argument of cons must be a list.");
	}

	sexpr *result = sexpr_list();
	sexpr_list_insert(result, resolve_sexp(env, nodes[1]));

	for (int j = 0; j < a2->count; j++) {
		sexpr_list_insert(result, sexpr_copy(a2->children[j]));
	}

	sexpr_free(a2);

	return result;
}

sexpr* builtin_nullq(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("null? expects just 1 argument");

	sexpr *a = resolve_sexp(env, nodes[1]);
	if (a->type == LVAL_ERR)
		return a;

	sexpr *result = sexpr_bool(a->type == LVAL_LIST && a->count == 0 ? 1 :0);

	sexpr_free(a);

	return result;
}

/* eq? as defined in the Little Schemer operates only on non-numeric atoms,
		but Scheme implementations I've seen accept broader inputs. I'm going to
		stick to the Little Schemer "standard" for now */
sexpr* builtin_eq(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3)
		return sexpr_err("eq? expects exactly 2 arguments.");

	sexpr *a = resolve_sexp(env, nodes[1]);
	sexpr *b = resolve_sexp(env, nodes[2]);

	/* Note, this actually hides errors that occurred in resolving the two
			inputs, which is maybe not the most ideal behaviour */
	sexpr *result = NULL;
	if (a->type == LVAL_BOOL && b->type == LVAL_BOOL && a->bool == b->bool)
		result = sexpr_bool(1);
	else if (a->type == LVAL_SYM && b->type == LVAL_SYM && strcmp(a->sym, b->sym) == 0)
		result = sexpr_bool(1);
	else
		result = sexpr_bool(0);

	sexpr_free(a);
	sexpr_free(b);

	return result;
}

sexpr* builtin_eval(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("Eval requires just one parameter.");

	sexpr *f = resolve_sexp(env, nodes[1]);

	if (IS_FUNC(f)) {
		sexpr *f2 = eval(env, f);
		sexpr_free(f);
		f = f2;
	}

	return f;
}

sexpr* builtin_op(scheme_env *env, sexpr **nodes, int count, sexpr *func) {
	if (func->type != LVAL_SYM)
		return sexpr_err("Expected symbol/built-in op!");

	char *op = func->sym;
	sexpr *result = NULL;

	/* I am not going to be fussy about the difference between integers
		and real numbers. If I am evaluating: + 1 2 14.0, then I'll just
		convert the result type to float when I hit the real number */
	if (strstr("+-*/%", op))
		result = builtin_math_op(env, nodes, count, op);

	if (strcmp(op, "min") == 0)
		result = builtin_min_op(env, nodes, count, op);

	if (strcmp(op, "max") == 0)
		return builtin_max_op(env, nodes, count, op);

	if (strcmp(op, "list") == 0)
		return builtin_list(env, nodes, count, op);

	if (strcmp(op, "car") == 0)
		return builtin_car(env, nodes, count, op);

	if (strcmp(op, "cdr") == 0)
		return builtin_cdr(env, nodes, count, op);

	if (strcmp(op, "cons") == 0)
		return builtin_cons(env, nodes, count, op);

	if (strcmp(op, "null?") == 0)
		return builtin_nullq(env, nodes, count, op);

	if (strcmp(op, "eq?") == 0)
		return builtin_eq(env, nodes, count, op);

	if (strcmp(op, "eval") == 0)
		return builtin_eval(env, nodes, count, op);

	if (strcmp(op, "self-test") == 0)
		return builtin_self_test(env, nodes, count, op);

	return result;
}

int is_built_in(sexpr *e) {
	if (e->type != LVAL_SYM)
		return 0;

	if (strstr("+-*/%", e->sym))
		return 1;
	else if (strcmp(e->sym, "max") == 0)
		return 1;
	else if (strcmp(e->sym, "min") == 0)
		return 1;
	else if (strcmp(e->sym, "list") == 0)
		return 1;
	else if (strcmp(e->sym, "car") == 0)
		return 1;
	else if (strcmp(e->sym, "cdr") == 0)
		return 1;
	else if (strcmp(e->sym, "cons") == 0)
		return 1;
	else if (strcmp(e->sym, "quote") == 0)
		return 1;
	else if (strcmp(e->sym, "define") == 0)
		return 1;
	else if (strcmp(e->sym, "null?") == 0)
		return 1;
	else if (strcmp(e->sym, "eq?") == 0)
		return 1;
	else if (strcmp(e->sym, "eval") == 0)
		return 1;
	else if (strcmp(e->sym, "self-test") == 0)
		return 1;

	return 0;
}

int is_quote_form(sexpr *e) {
	if (e->type != LVAL_SYM || strcmp(e->sym, "quote") != 0)
		return 0;

	return 1;
}

sexpr* define_var(scheme_env *env, sexpr *exp) {
	if (exp->count != 3)
		return sexpr_err("Define requires a name and a definition.");

	if (exp->children[1]->type != LVAL_SYM)
		return sexpr_err("Expected variable name.");

	/* Actually I don't actually know if you can redefine built-ins in Scheme... */
	if (is_built_in(exp->children[1]))
		return sexpr_err("Cannot redefine built-in function.");

	sexpr *result = resolve_sexp(env, exp->children[2]);
	if (result->type == LVAL_ERR)
		return result;

	env_insert_var(env, exp->children[1]->sym, result);

	return sexpr_null();
}

sexpr* eval(scheme_env *env, sexpr *v) {
	sexpr *result = NULL;
	switch (v->type) {
		case LVAL_LIST:
			if (v->count == 0)
				return sexpr_err("Expected operator or function");

			/* What function are we trying to execute? It might be a built-in, or it
				could a stored value we need to evaluate, for instance:

				(define f 'list)
				(eval '((eval f) 1 2))
			*/
			sexpr *func;
			if (is_built_in(v->children[0]))
				func = sexpr_copy(v->children[0]);
			else
				func = resolve_sexp(env, v->children[0]);

			if (!is_built_in(func)) {
				sexpr_free(func);
				return sexpr_err("Expected operator or function");
			}

			/* quote is a special form -- we simply return its first paramter without evaluating it.
				Note that I am not certain I really understand quote, so I'm mostly going by what's in
				the Little Schemer and mucking around with other Scheme REPLs.

				For instance (quote 1 2 3) returns 1 but I'm not sure if that's valid input from a proper
				definition of Scheme or not.
				*/
			if (is_quote_form(func)) {
				return sexpr_copy(v->children[1]);
			}

			if (strcmp(func->sym, "define") == 0) {
				/* We are defining either a variable or a function. */
				result = define_var(env, v);

				sexpr_free(func);
				return result;
			}

			result = builtin_op(env, v->children, v->count, func);
			sexpr_free(func);

			return result;
		case LVAL_SYM:
			/* Is the symbol a variable? */
			result = env_fetch_var(env, v->sym);
			return result;
			break;
		case LVAL_ERR:
			return v;
			break;
		case LVAL_NUM:
		case LVAL_BOOL:
		case LVAL_NULL:
			return sexpr_copy(v);
	}

	return sexpr_err("Something hasn't been implemented yet");
}
