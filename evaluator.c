#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "environment.h"
#include "sexpr.h"
#include "parser.h"
#include "util.h"

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
		case LVAL_FUN:
			if (s1->fun != s2->fun)
				return 0;
			break;
		case LVAL_NULL:
			return 1;
	}

	return 1;
}

int test_sexpr(scheme_env *env, char *s, sexpr *expected) {
	int c = 0;
	char *line = NULL;
	line = n_strcpy(line, s);
	sexpr *ast = parse(line, &c);
	printf("Checking: %s\n", line);
	sexpr *result = eval2(env, ast);

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
	load_built_ins(test_env);
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
	sexpr *expected = eval2(test_env, p);
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

sexpr *builtin_not(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("Just one parameter expected.");

	sexpr *v = eval2(env, nodes[1]);
	if (v->type != LVAL_BOOL)
		return sexpr_err("Boolean value expected.");

	sexpr *result = sexpr_bool(!v->bool);
	sexpr_free(v);

	return result;
}

/* I find Scheme's version of and pretty odd in how all non-booleans
	are considered true. */
sexpr* builtin_and(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(1);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			sexpr_free(result);
			return cp;
		}

		if (cp->type == LVAL_BOOL && !cp->bool)
			return sexpr_bool(0);

		result = sexpr_copy(cp);
		sexpr_free(cp);
	}

	return result;
}

sexpr* builtin_or(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(0);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			sexpr_free(result);
			return cp;
		}

		if (cp->type != LVAL_BOOL || (cp->type == LVAL_BOOL && cp->bool))
			return cp;

		result = sexpr_copy(cp);
		sexpr_free(cp);
	}

	return result;
}

sexpr *builtin_math_eq(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(env, nodes[1]);
	sexpr *n1 = eval2(env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("= operators on numbers or bools.");

	int eq = 0;
	if (n0->num_type == NUM_TYPE_INT && n1->num_type == NUM_TYPE_INT)
		eq = n0->n.i_num == n1->n.i_num;
	else if (n0->num_type == NUM_TYPE_DEC && n1->num_type == NUM_TYPE_DEC)
		eq = fabsf(n0->n.d_num - n1->n.d_num) < 0.00000001;

	sexpr_free(n0);
	sexpr_free(n1);

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr *builtin_math_gt(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(env, nodes[1]);
	sexpr *n1 = eval2(env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("= operators on numbers or bools.");

	int eq = 0;
	if (n0->num_type == NUM_TYPE_INT && n1->num_type == NUM_TYPE_INT)
		eq = n0->n.i_num > n1->n.i_num;
	else if (n0->num_type == NUM_TYPE_DEC || n1->num_type == NUM_TYPE_DEC) {
		float f0, f1;
		f0 = n0->num_type == NUM_TYPE_INT ? n0->n.i_num : n0->n.d_num;
		f1 = n1->num_type == NUM_TYPE_INT ? n1->n.i_num : n1->n.d_num;
		eq = f0 > f1;
	}

	sexpr_free(n0);
	sexpr_free(n1);

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr *builtin_math_lt(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(env, nodes[1]);
	sexpr *n1 = eval2(env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("= operators on numbers or bools.");

	int eq = 0;
	if (n0->num_type == NUM_TYPE_INT && n1->num_type == NUM_TYPE_INT)
		eq = n0->n.i_num < n1->n.i_num;
	else if (n0->num_type == NUM_TYPE_DEC || n1->num_type == NUM_TYPE_DEC) {
		float f0, f1;
		f0 = n0->num_type == NUM_TYPE_INT ? n0->n.i_num : n0->n.d_num;
		f1 = n1->num_type == NUM_TYPE_INT ? n1->n.i_num : n1->n.d_num;
		eq = f0 < f1;
	}

	sexpr_free(n0);
	sexpr_free(n1);

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr *builtin_math_gte(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(env, nodes[1]);
	sexpr *n1 = eval2(env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("= operators on numbers or bools.");

	int eq = 0;
	if (n0->num_type == NUM_TYPE_INT && n1->num_type == NUM_TYPE_INT)
		eq = n0->n.i_num >= n1->n.i_num;
	else if (n0->num_type == NUM_TYPE_DEC || n1->num_type == NUM_TYPE_DEC) {
		float f0, f1;
		f0 = n0->num_type == NUM_TYPE_INT ? n0->n.i_num : n0->n.d_num;
		f1 = n1->num_type == NUM_TYPE_INT ? n1->n.i_num : n1->n.d_num;
		eq = f0 >= f1;
	}

	sexpr_free(n0);
	sexpr_free(n1);

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr *builtin_math_lte(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(env, nodes[1]);
	sexpr *n1 = eval2(env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("= operators on numbers or bools.");

	int eq = 0;
	if (n0->num_type == NUM_TYPE_INT && n1->num_type == NUM_TYPE_INT)
		eq = n0->n.i_num <= n1->n.i_num;
	else if (n0->num_type == NUM_TYPE_DEC || n1->num_type == NUM_TYPE_DEC) {
		float f0, f1;
		f0 = n0->num_type == NUM_TYPE_INT ? n0->n.i_num : n0->n.d_num;
		f1 = n1->num_type == NUM_TYPE_INT ? n1->n.i_num : n1->n.d_num;
		eq = f0 <= f1;
	}

	sexpr_free(n0);
	sexpr_free(n1);

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr* builtin_math_op(scheme_env *env, sexpr **nodes, int count, char *op) {
	sexpr *result = NULL;

	/* Unary subtraction */
	if (strcmp(op, "-") == 0 && count == 2) {
		sexpr *n = eval2(env, nodes[1]);
		if (n->type != LVAL_NUM)
			result = sexpr_err("Expected number!");
		else {
			result = sexpr_num_s("0");
			sexpr_num_sub(result, nodes[1]);
		}
		sexpr_free(n);

		return result;
	}

	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(env, nodes[j]);

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
		sexpr *n = eval2(env, nodes[j]);
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
		sexpr *n = eval2(env, nodes[j]);
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
		sexpr *cp = eval2(env, nodes[j]);
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

	sexpr *l = eval2(env, nodes[1]);
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

	sexpr *l = eval2(env, nodes[1]);

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

	sexpr *a2 = eval2(env, nodes[2]);

	if (a2->type != LVAL_LIST) {
		sexpr_free(a2);
		return sexpr_err("The second argument of cons must be a list.");
	}

	sexpr *result = sexpr_list();
	sexpr_list_insert(result, eval2(env, nodes[1]));

	for (int j = 0; j < a2->count; j++) {
		sexpr_list_insert(result, sexpr_copy(a2->children[j]));
	}

	sexpr_free(a2);

	return result;
}

sexpr* builtin_nullq(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("null? expects just 1 argument");

	sexpr *a = eval2(env, nodes[1]);
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

	sexpr *a = eval2(env, nodes[1]);
	sexpr *b = eval2(env, nodes[2]);

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

	sexpr *f = eval2(env, nodes[1]);

	if (IS_FUNC(f)) {
		sexpr *f2 = eval2(env, f);
		sexpr_free(f);
		f = f2;
	}

	return f;
}

sexpr* quote_form(scheme_env *env, sexpr **nodes, int count, char *op) {
	return sexpr_copy(nodes[1]);
}

int is_quoted_val(sexpr *v) {
	if (v->type != LVAL_LIST || v->count == 0)
		return 0;

	sexpr *c = v->children[0];
	if (c->type == LVAL_SYM && strcmp(c->sym, "quote") == 0)
		return 1;

	return 0;
}

sexpr* define_var(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (nodes[1]->type != LVAL_SYM)
		return sexpr_err("Expected variable name.");

	ASSERT_PRIMITIVE(env, nodes[1]->sym);

	if (is_quoted_val(nodes[2]))
		env_insert_var(env, nodes[1]->sym, sexpr_copy(nodes[2]->children[1]));
	else
		env_insert_var(env, nodes[1]->sym, eval2(env, nodes[2]));
	
	return sexpr_null();
}

sexpr* define_fun(scheme_env *env, sexpr **nodes, int count, char *op)  {
	sexpr *header = nodes[1];
	sexpr *body = nodes[2];

	if (header->count == 0 || body->count == 0)
		return sexpr_err("Invalid definition.");

	ASSERT_PRIMITIVE(env, header->children[0]->sym);
	char *fun_num = header->children[0]->sym;

	/* Each parameter must be a symbol and be uniquely named
		Note to self: there can be zero params of course */
	sexpr *params = sexpr_list();
	for (int j = 1; j < header->count; j++) {
		if (header->children[j]->type != LVAL_SYM) {
			sexpr_free(params);
			return sexpr_err("Paramter names must be symbols.");
		}
		sexpr_append(params, sexpr_copy(header->children[j]));
	}

	sexpr *fun = sexpr_fun_user(params, sexpr_copy(body), fun_num);
	env_insert_var(env, fun_num, fun);

	return sexpr_null();
}

sexpr* define(scheme_env *env, sexpr **nodes, int count, char *op) {
	if (count != 3)
		return sexpr_err("Invalid definition.");

	if (nodes[1]->type == LVAL_LIST)
		return define_fun(env, nodes, count, op);
	else
		return define_var(env, nodes, count, op);
}

sexpr* resolve_symbol(scheme_env *env, sexpr *s) {
	sexpr *r = env_fetch_var(env, s->sym);
	if (r->type == LVAL_SYM) {
		sexpr *r2 = resolve_symbol(env, r);
		sexpr_free(r);
		return r2;
	}

	return r;
}

sexpr* eval_user_func(scheme_env *env, sexpr **operands, int count, sexpr *fun) {
	if ((count - 1) < fun->params->count) {
		return sexpr_err("Too few paramters passed to function.");
	}

	scheme_env *func_scope = env_new();
	func_scope->parent = env;

	/* Map the operands to the function parameters and add them to the
	 	local scope */
	for (int j = 0; j < fun->params->count; j++) {
		sexpr *var;
		if (operands[j + 1]->type == LVAL_SYM)
			var = resolve_symbol(env, operands[j + 1]);
		else
			var = sexpr_copy(operands[j + 1]);

		env_insert_var(func_scope, fun->params->children[j]->sym, var);
	}

	sexpr *result = eval2(func_scope, fun->body);

	env_free(func_scope);
	return result;
}

sexpr* eval2(scheme_env *env, sexpr *v) {
	sexpr *result = NULL;
	switch (v->type) {
		case LVAL_LIST:
			if (v->count == 0)
				return sexpr_err("Expected operator or function");

			sexpr *func;
			if (v->children[0]->type == LVAL_LIST)
				func = eval2(env, v->children[0]);
			else if (v->children[0]->type == LVAL_SYM)
				func = resolve_symbol(env, v->children[0]);
			else
				return sexpr_err("Expected function!!");

			if (func->type != LVAL_FUN) {
				if (func->type == LVAL_ERR)
					return func;
				else {
					sexpr_free(func);
					return sexpr_err("Expected function.");
				}
			}

			/* Okay! We have our function! Is it a primitive or user-defined? */
			if (func->builtin)
				result = func->fun(env, v->children, v->count, func->sym);
			else
				result = eval_user_func(env, v->children, v->count, func);

			sexpr_free(func);

			return result;
		case LVAL_SYM:
			result = resolve_symbol(env, v);
			return result;
		case LVAL_FUN:
			break;
		case LVAL_ERR:
			return v;
		case LVAL_NUM:
		case LVAL_BOOL:
		case LVAL_NULL:
			return sexpr_copy(v);
			break;
	}

	return sexpr_err("Something hasn't been implemented yet");
}

void load_built_ins(scheme_env *env) {
	env_insert_var(env, "car", sexpr_fun_builtin(&builtin_car, "car"));
	env_insert_var(env, "cdr", sexpr_fun_builtin(&builtin_cdr, "cdr"));
	env_insert_var(env, "cons", sexpr_fun_builtin(&builtin_cons, "cons"));
	env_insert_var(env, "list", sexpr_fun_builtin(&builtin_list, "list"));
	env_insert_var(env, "eq?", sexpr_fun_builtin(&builtin_eq, "eq?"));
	env_insert_var(env, "null?", sexpr_fun_builtin(&builtin_nullq, "null?"));
	env_insert_var(env, "eval", sexpr_fun_builtin(&builtin_eval, "eval"));
	env_insert_var(env, "self-test", sexpr_fun_builtin(&builtin_self_test, "self-test"));
	env_insert_var(env, "+", sexpr_fun_builtin(&builtin_math_op, "+"));
	env_insert_var(env, "-", sexpr_fun_builtin(&builtin_math_op, "-"));
	env_insert_var(env, "*", sexpr_fun_builtin(&builtin_math_op, "*"));
	env_insert_var(env, "/", sexpr_fun_builtin(&builtin_math_op, "/"));
	env_insert_var(env, "%", sexpr_fun_builtin(&builtin_math_op, "%"));
	env_insert_var(env, "=", sexpr_fun_builtin(&builtin_math_eq, "="));
	env_insert_var(env, ">", sexpr_fun_builtin(&builtin_math_gt, ">"));
	env_insert_var(env, ">=", sexpr_fun_builtin(&builtin_math_gte, ">="));
	env_insert_var(env, "<", sexpr_fun_builtin(&builtin_math_lt, "<"));
	env_insert_var(env, "<=", sexpr_fun_builtin(&builtin_math_lte, "<="));
	env_insert_var(env, "not", sexpr_fun_builtin(&builtin_not, "not"));
	env_insert_var(env, "or", sexpr_fun_builtin(&builtin_or, "or"));
	env_insert_var(env, "and", sexpr_fun_builtin(&builtin_and, "and"));
	env_insert_var(env, "min", sexpr_fun_builtin(&builtin_min_op, "min"));
	env_insert_var(env, "max", sexpr_fun_builtin(&builtin_max_op, "max"));
	env_insert_var(env, "define", sexpr_fun_builtin(&define, "define"));
	env_insert_var(env, "quote", sexpr_fun_builtin(&quote_form, "quote"));
}
