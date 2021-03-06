#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "environment.h"
#include "sexpr.h"
#include "parser.h"
#include "util.h"

#define CLOSURE_TABLE_SIZE 47

/* I need variable names for things like closures. They need to be unique and
	they are only used internally so integers should work fine. (An integer
	isn't an invalid symbol name so they should never conflict with other
	bound symbols */
sexpr* gen_private_var_name(vm_heap *vm, scope *sc) {
	char buffer[50];
	sexpr *e = sexpr_null();

	while (e->type != LVAL_ERR) {
		int n = rand();
		sprintf(buffer, "%d", n);
		e = scope_fetch_var(vm, sc, buffer);
	}

	return sexpr_sym(vm, buffer);
}

int is_zero(sexpr *num) {
	if (num->num_type == NUM_TYPE_INT)
		return num->i_num == 0;
	else
		return (fabs(0 - num->d_num) < 0.00000001);
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
			if (s1->num_type == NUM_TYPE_INT && s1->i_num != s2->i_num)
				return 0;
			if (s1->num_type == NUM_TYPE_DEC && s1->d_num != s2->d_num)
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
		case LVAL_STR:
			return strcmp(s1->str, s2->str) == 0 ? 1 : 0;
		case LVAL_NULL:
			return 1;
	}

	return 1;
}

/* Read in a file of Scheme source code. It expects one parameter -- the
	file name as a string. I guess the parameter could be a calculated/evaled
	string but I can't imagine myself needing that over the course of working
	through the Little Schemer

	Note: it's actually possible to call load in a nested function. Eg:
		(define f (lambda () (load "atomq.scm")))

	I'm not sure that's a problem or not. The functions loaded and variables
	bound will disappear once that scope is exited.

	I could always seek out the global scope when load is called, but tbh
	I'm not sure which behaviour is correct/better.
*/
sexpr* builtin_load(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "Load expects only the filename to be loaded.");
	ASSERT_TYPE(nodes[1], LVAL_STR, "Filename must be a string constant.");

    tokenizer *tk = tokenizer_new();
	if (!start_file(tk, nodes[1]->str)) {
		tokenizer_free(tk);
		return sexpr_err(vm, "File not found.");
	}

	parser *p = parser_new(tk);
	sexpr* ast = get_next_expr(vm, p);
    while (ast->type != LVAL_NULL)
    {
		sexpr *result = eval2(vm, env, ast);
		if (result->type != LVAL_NULL) {
			sexpr_pprint(result);
			putchar('\n');
		}

		ast = get_next_expr(vm, p);
    }

	tokenizer_free(tk);
	parser_free(p);

	return sexpr_null();
}

sexpr* builtin_mem_dump(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	env_dump(vm, env);

	return sexpr_null();
}

/* I think this is probably incorrect, or not totally correct because I haven't
	yet learned about pairs in Scheme yet. But in the REPLs I've tried,
	pair? returns false for atoms or an empty list */
sexpr *builtin_pairq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "Just one parameter expected.");

	int pq = 0;
	sexpr *v = eval2(vm, env, nodes[1]);
	if (v->type == LVAL_LIST && v->count > 0)
		pq = 1;

	return sexpr_bool(vm, pq);
}

sexpr *builtin_not(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "Just one parameter expected.");

	sexpr *v = eval2(vm, env, nodes[1]);
	ASSERT_TYPE(v, LVAL_BOOL, "Boolean value expected.");

	return sexpr_bool(vm, !v->bool);
}

/* I find Scheme's version of and pretty odd in how all non-booleans
	are considered true. */
sexpr* builtin_and(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = NULL;

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(vm, 1);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			return cp;
		}

		if (cp->type == LVAL_BOOL && !cp->bool)
			return sexpr_bool(vm, 0);

		result = cp;
	}

	return result;
}

sexpr* builtin_or(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = NULL;

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(vm, 0);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			return cp;
		}

		if (cp->type != LVAL_BOOL || (cp->type == LVAL_BOOL && cp->bool))
			return cp;

		result = cp;
	}

	return result;
}

/* Converting integers into doubles and then comparing them that way is
	probably deeply flawed, but the code sure is cleaner than trying to sort
	out if we are comparing ints to ints, doubles to doubles or a mix.

	It should be sufficiently fast and accurate for anything I might want to
	use notion for.
 */
sexpr* builtin_math_cmp(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "Just two parameters expected.");

	sexpr *n0 = eval2(vm, env, nodes[1]);
	sexpr *n1 = eval2(vm, env, nodes[2]);

	ASSERT_TYPE(n0, LVAL_NUM, "Number expected.");
	ASSERT_TYPE(n1, LVAL_NUM, "Number expected.");

	int eq = 0;
	double f0 = n0->num_type == NUM_TYPE_INT ? n0->i_num : n0->d_num;
	double f1 = n1->num_type == NUM_TYPE_INT ? n1->i_num : n1->d_num;

	if (strcmp("=", op) == 0)
		eq = fabs(f0 - f1) < 0.000000001;
	else if (strcmp("<", op) == 0)
		eq = f0 < f1;
	else if (strcmp(">", op) == 0)
		eq = f0 > f1;
	else if (strcmp(">=", op) == 0)
		eq = f0 > f1 || fabs(f0 - f1) < 0.000000001;
	else if (strcmp("<=", op) == 0)
		eq = f0 < f1 || fabs(f0 - f1) < 0.000000001;

	return eq ? sexpr_bool(vm, 1) : sexpr_bool(vm, 0);
}

sexpr* builtin_math_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	double result;
	sexpr *r;

	/* Unary subtraction */
	if (strcmp(op, "-") == 0 && count == 2) {
		sexpr *n = eval2(vm, env, nodes[1]);
		ASSERT_TYPE(n, LVAL_NUM, "Expected number!");

		result = - (NUM_CONVERT(n));
		return sexpr_num(vm, n->num_type, result);
	}

	enum sexpr_num_type rt = NUM_TYPE_INT;
	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);
		ASSERT_TYPE(n, LVAL_NUM, "Expected number!");

		if (n->num_type == NUM_TYPE_DEC)
			rt = NUM_TYPE_DEC;

		/* The first number value after the operator is result's starting value */
		if (j == 1) {
			result = NUM_CONVERT(n);
			continue;
		}

		if (strcmp(op, "+") == 0)
			result += NUM_CONVERT(n);
		else if (strcmp(op, "-") == 0) 
			result -= NUM_CONVERT(n);
		else if (strcmp(op, "*") == 0)
			result *= NUM_CONVERT(n);
		else if (strcmp(op, "/") == 0) {
			if (is_zero(n))
				return sexpr_err(vm, "Division by zero!");

			
			result /= NUM_CONVERT(n);

			// If the result contains a fractional component, force
			// the result to be decimal type. notion probably shouldn't
			// be used for precision financial or scientific calculations...
			long temp_r = result;
			if (fabs(result - temp_r) > 0.00000000001)
				rt = NUM_TYPE_DEC;
		}
		else if (strcmp(op, "^") == 0) {
			result = pow(result,NUM_CONVERT(n));
		}
	}

	r = sexpr_num(vm, rt, result);

	return r;
}

sexpr* builtin_math_modulo(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "Modoulo takes exactly two parameters.");

	sexpr *dividend = eval2(vm, env, nodes[1]);
	sexpr *divisor = eval2(vm, env, nodes[2]);
	ASSERT_TYPE(dividend, LVAL_NUM, "The dividend must be an integer");
	ASSERT_TYPE(divisor, LVAL_NUM, "The divisor must be an integer");

	if (dividend->num_type != NUM_TYPE_INT || divisor->num_type != NUM_TYPE_INT) {
		return sexpr_err(vm, "Can only calculate the remainder for integers.");
	}
	else if (is_zero(divisor)) {
		return sexpr_err(vm, "Division by zero!");
	}

	long result = dividend->i_num % divisor->i_num;

	return sexpr_num(vm, NUM_TYPE_INT, result);
}

sexpr* builtin_min_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 2, "At least one parameter needed for min");

	double curr_max, x;

	enum sexpr_num_type rt = NUM_TYPE_INT;
	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);
		ASSERT_TYPE(n, LVAL_NUM, "Expected number!");

		if (n->num_type == NUM_TYPE_DEC)
			rt = NUM_TYPE_DEC;

		if (j == 1) {
			curr_max = NUM_CONVERT(n);
			continue;
		}

		x = NUM_CONVERT(n);
		if (x < curr_max)
			curr_max = x;
	}

	return sexpr_num(vm, rt, curr_max);
}

sexpr* builtin_max_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 2, "At least one parameter needed for max");

	double curr_max, x;
	enum sexpr_num_type rt = NUM_TYPE_INT;

	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);
		ASSERT_TYPE(n, LVAL_NUM, "Expected number!");

		if (n->num_type == NUM_TYPE_DEC)
			rt = NUM_TYPE_DEC;

		if (j == 1) {
			curr_max = NUM_CONVERT(n);
			continue;
		}

		x = NUM_CONVERT(n);
		if (x > curr_max)
			curr_max = x;
	}

	return sexpr_num(vm, rt, curr_max);
}

sexpr* builtin_list(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list(vm);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		ASSERT_NOT_ERR(cp);
		sexpr_append(result, cp);
	}

	return result;
}

sexpr* builtin_cdr(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "cdr expects only one argument");

	sexpr *l = eval2(vm, env, nodes[1]);
	if (l->type != LVAL_LIST || l->count == 0) {
		return sexpr_err(vm, "cdr is defined only for non-empty lists.");
	}

	sexpr *result = sexpr_list(vm);
	for (int j = 1; j < l->count; j++) {
		sexpr_append(result, l->children[j]);
	}

	return result;
}

sexpr* builtin_car(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "car expects only one argument");

	sexpr *l = eval2(vm, env, nodes[1]);

	if (l->type != LVAL_LIST || l->count == 0) {
		return sexpr_err(vm, "car is defined only for non-empty lists.");
	}

	return l->children[0];
}

sexpr* builtin_cons(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "cons expects two aruments");

	sexpr *a2 = eval2(vm, env, nodes[2]);

	ASSERT_NOT_ERR(a2);
	ASSERT_TYPE(a2, LVAL_LIST, "The second argument of cons must be a list.");

	sexpr *result = sexpr_list(vm);
	sexpr_append(result, eval2(vm, env, nodes[1]));

	for (int j = 0; j < a2->count; j++) {
		sexpr_append(result, a2->children[j]);
	}

	return result;
}

sexpr* builtin_nullq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "null? expects just 1 argument");

	sexpr *a = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(a);

	return sexpr_bool(vm, a->type == LVAL_LIST && a->count == 0 ? 1 :0);
}

sexpr* builtin_numberq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "number? expects just 1 argument");

	sexpr *n = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(n);

	return sexpr_bool(vm, n->type == LVAL_NUM);
}

/* eq? as defined in the Little Schemer operates only on non-numeric atoms,
		but Scheme implementations I've seen accept broader inputs. I'm going to
		stick to the Little Schemer "standard" for now */
sexpr* builtin_eq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "eq? expects exactly 2 arguments.");

	sexpr *a = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(a);
	sexpr *b = eval2(vm, env, nodes[2]);
	ASSERT_NOT_ERR(b);

	if (a->type == LVAL_BOOL && b->type == LVAL_BOOL && a->bool == b->bool)
		return sexpr_bool(vm, 1);
	else if (a->type == LVAL_SYM && b->type == LVAL_SYM && strcmp(a->sym, b->sym) == 0)
		return sexpr_bool(vm, 1);
	else if (a->type == LVAL_STR && b->type == LVAL_STR && strcmp(a->str, b->str) == 0)
		return sexpr_bool(vm, 1);
	else if (a->type == LVAL_NUM && b->type == LVAL_NUM && a->num_type == b->num_type) {
		if (a->num_type == NUM_TYPE_INT && a->i_num == b->i_num)
			return sexpr_bool(vm, 1);
		else if (a->num_type == NUM_TYPE_DEC && fabs(a->d_num - b->d_num) < 0.0000001)
			return sexpr_bool(vm, 1);
		else
			return sexpr_bool(vm, 0);
	}
	else if (a == b)
		return sexpr_bool(vm, 1);
	else
		return sexpr_bool(vm, 0);
}

sexpr* builtin_eval(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "eval expects just 1 argument");

	sexpr *f = eval2(vm, env, nodes[1]);
	f = eval2(vm, env, f);

	return f;
}

sexpr* builtin_quit(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	return sexpr_err(vm, "<quit>");
}

sexpr* quote_form(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	return nodes[1];
}

int is_quoted_val(sexpr *v) {
	if (v->type != LVAL_LIST || v->count == 0)
		return 0;

	sexpr *c = v->children[0];
	if (c->type == LVAL_SYM && strcmp(c->sym, "quote") == 0)
		return 1;

	return 0;
}

sexpr* define_var(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op) {
	ASSERT_TYPE(nodes[1], LVAL_SYM, "Expected variable name.");
	ASSERT_PRIMITIVE(vm, sc, nodes[1]->sym);

	if (is_quoted_val(nodes[2]))
		scope_insert_var(sc, nodes[1]->sym, nodes[2]->children[1]);
	else
		scope_insert_var(sc, nodes[1]->sym, eval2(vm, sc, nodes[2]));

	return sexpr_null();
}

sexpr* build_func_stmt(vm_heap *vm, sexpr *header, sexpr *body, char *name) {
	/* Each parameter must be a symbol and be uniquely named
		Note to self: there can be zero params of course */
	sexpr *params = sexpr_list(vm);
	for (int j = 1; j < header->count; j++) {
		ASSERT_TYPE(header->children[j], LVAL_SYM, "Paramter names must be symbols.");
		sexpr_append(params, header->children[j]);
	}

	return sexpr_fun_user(vm, params, body, name);
}

sexpr* builtin_if(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 4, "If is of the form (if <pred> <consequent> <alternate>.");

	sexpr *result = eval2(vm, env, nodes[1]);
	if (result->type == LVAL_ERR)
		return result;
	else if (result->type == LVAL_BOOL) {
		return result->bool ? eval2(vm, env, nodes[2]) : eval2(vm, env, nodes[3]);
	}
	else {
		// Evidently, a non-boolean value is considered true so:
		// (if (+ 1 2 3) 17 8) would have 17 for a result
		return eval2(vm, env, nodes[2]);
	}
}

sexpr* builtin_cond(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 2, "Cond requires at least one expression.");

	for (int j = 1; j < count; j++) {
		/* Each item in the cond expression must be a list containing a test
			and a result. If the test resolves to neither #t or #f then we have
			an error condition.

			An easy way to do the else clause is to make it a function that
			always returns true, but the else clause can only be the final item.
		*/
		if (nodes[j]->type == LVAL_LIST) {
			sexpr *cond = nodes[j];
			ASSERT_PARAM_EQ(cond->count, 2, "Invalid cond expression.");

			if (IS_ELSE_CLAUSE(j, count, cond->children[0]))
			{
				return eval2(vm, env, cond->children[1]);
			}

			sexpr *result = eval2(vm, env, cond->children[0]);
			if (result->type == LVAL_ERR)
				return result;
			else if (result->type != LVAL_BOOL) {
				return sexpr_err(vm, "Invalid boolean test.");
			}

			if (result->bool)
				return eval2(vm, env, cond->children[1]);
		}
		else
			return sexpr_err(vm, "Cond tests must be an expression.");
	}

	return sexpr_null();
}

sexpr* builtin_stringq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String? takes just one paramter.");

	sexpr *s = eval2(vm, env, nodes[1]);

	return sexpr_bool(vm, s->type == LVAL_STR ? 1 : 0);
}

sexpr* builtin_stringlen(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String? takes just one paramter.");

	sexpr *s = eval2(vm, env, nodes[1]);
	ASSERT_TYPE(s, LVAL_STR, "That was not a string.");

	return sexpr_num(vm, NUM_TYPE_INT, strlen(s->str));
}

sexpr* builtin_string(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String takes just one parameter.");

	sexpr *src = eval2(vm, env, nodes[1]);
	ASSERT_TYPE(src, LVAL_STR,  "String takes a string type for its parameter.");

	return src;
}

sexpr* builtin_stringappend(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "String-append takse just two paramters");

	sexpr *s1 = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(s1);
	sexpr *s2 = eval2(vm, env, nodes[2]);
	ASSERT_NOT_ERR(s2);

	sexpr *result;
	if (s1->type != LVAL_STR || s2->type != LVAL_STR) {
		result = sexpr_err(vm, "String-append requiers two strings.");
	}
	else {
		/* s1 and s2 are strings, but their values could be NULL */
		int len = 1 + (s1 ? strlen(s1->str) : 0);
		len += s2 ? strlen(s2->str) : 0;

		/* Competent C coders are probably recoiling in horror
			from this next bit... */
		result = sexpr_str(vm, NULL);
		result->str = malloc(len);
		result->str[len - 1] = '\0';

		int j = 0;
		char *c = s1->str;
		while (c && *c != '\0') {
			result->str[j++] = *c;
			++c;
		}
		c = s2->str;
		while (c && *c != '\0') {
			result->str[j++] = *c;
			++c;
		}
	}

	return result;
}

sexpr* builtin_stringcopy(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String-append takse just one paramter");

	sexpr *s1 = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(s1);
	ASSERT_TYPE(s1, LVAL_STR, "String copy only copies strings.");

	return sexpr_copy(vm, s1);
}

int is_local_param(sexpr *params, sexpr* sym) {
	for (int j = 0; j < params->count; j++) {
		sexpr *p = params->children[j];
		if (p->sym && sym->sym && strcmp(p->sym, sym->sym) == 0)
			return 1;
	}

	return 0;
}

/* Scan for any values that are going to become unbound and thus I need to keep
	copies of. For instance, the parameters of nested lambda calls. If I find
	a value stored in a local scope, replace the symbol in the expression with
	a randomly generated, unique variable name. Then, it lives on the global
	scope as long as the function exists.
 */
sexpr* scan_for_closures(vm_heap *vm, scope *env, sexpr *params, sexpr *body) {
	for (int j = 0; j < body->count; j++) {
		sexpr *var = body->children[j];

		if (var->type == LVAL_SYM) {
			if (is_local_param(params, var))
				continue;

			sexpr *f = scope_fetch_var(vm, env, var->sym);
			if (f->type != LVAL_ERR && !f->global_scope) {
				sexpr *cv = gen_private_var_name(vm, env);
				scope_insert_global_var(env, cv->sym, f);
				body->children[j] = cv;
			}
		}
		else if (var->type == LVAL_LIST) {
			body->children[j] = scan_for_closures(vm, env, params, var);
		}
	}

	return body;
}

sexpr* builtin_lambda(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "Invalid lambda definition.");

	/* So function evaluation excepts the paramter list for function
		calls to include the function name, so gotta add in a dummy
		value for lambdas */
	sexpr *params = sexpr_list(vm);
	sexpr_append(params, sexpr_null());
	for (int j = 0; j < nodes[1]->count; j++)
		sexpr_append(params, sexpr_copy(vm, nodes[1]->children[j]));
	sexpr *body = sexpr_copy(vm, nodes[2]);

	if (params->count == 0)
		return sexpr_err(vm, "Invalid definition.");

	body = scan_for_closures(vm, env, params, body);

	sexpr *lambda = build_func_stmt(vm, params, body, "");

	return lambda;
}

sexpr* define_fun(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op)  {
	sexpr *header = nodes[1];

	ASSERT_PRIMITIVE(vm, sc, header->children[0]->sym);
	char *fun_name = header->children[0]->sym;
	sexpr *fun;	

	for (int j = 2; j < count; j++ ) {
		sexpr *child = nodes[j]->children[0];	
		if (child->type == LVAL_SYM && strcmp(child->sym, "define") == 0) 
			fun = define_fun(vm, sc, nodes[j]->children, nodes[j]->count, op);
		else 
			fun = build_func_stmt(vm, header, nodes[j], fun_name);
			
		if (fun->type == LVAL_ERR)
			return fun;
	}

	scope_insert_var(sc, fun_name, fun);

	return sexpr_null();
}

sexpr* define(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 3, "Invalid definition.");

	// I need to change how I process define. If the first child node is a symbol,
	// we are defining a variable, otherwise we are trying to define a function
	// and that can have multiple statements, whereas I originally assumed it would 
	// only be: (define (name <params>) (<body>))
	if (nodes[1]->type == LVAL_SYM)
		return define_var(vm, sc, nodes, count, op);
	else if (nodes[1]->type == LVAL_LIST)
		return define_fun(vm, sc, nodes, count, op);
	else 
		return sexpr_err(vm, "Define: symbol or list expected.");	
}

sexpr* eval_user_func(vm_heap *vm, scope *sc, sexpr **operands, int count, sexpr *fun) {
	ASSERT_PARAM_MIN(count - 1, fun->params->count, "Too few paramters passed to function.");

	scope *func_scope = scope_new(CLOSURE_TABLE_SIZE);
	func_scope->parent = sc;

	/* Map the operands to the function parameters and add them to the
	 	local scope */
	for (int j = 0; j < fun->params->count; j++) {
		sexpr *var;
		if (operands[j + 1]->type == LVAL_SYM)
			var = scope_fetch_var(vm, sc, operands[j + 1]->sym);
		else if (operands[j + 1]->type == LVAL_LIST)
			var = eval2(vm, sc, operands[j + 1]);
		else
			var = operands[j + 1];

		if (var->type == LVAL_ERR)
			return var;

		scope_insert_var(func_scope, fun->params->children[j]->sym, var);
	}

	sexpr *result = eval2(vm, func_scope, fun->body);
	scope_free(func_scope);

	return result;
}

sexpr* eval2(vm_heap *vm, scope *sc, sexpr *v) {
	sexpr *result = NULL;

	switch (v->type) {
		case LVAL_LIST:
			/* An empty list evals to an empty list */
			if (v->count == 0)
				return sexpr_list(vm);

			sexpr *func = sexpr_null();
			if (v->children[0]->type == LVAL_SYM) {
				func = scope_fetch_var(vm, sc, v->children[0]->sym);
				while (func->type == LVAL_SYM)
					func = scope_fetch_var(vm, sc, func->sym);
			}
			else if (v->children[0]->type == LVAL_LIST) {
				func = eval2(vm, sc, v->children[0]);
			}

			if (func->type != LVAL_FUN) {
				char msg[1024];
				char *desc = sexpr_desc(func);
				snprintf(msg, sizeof msg, "%s%s", "Expected function. Instead got: ", desc);
				sexpr *err = sexpr_err(vm, msg);
				free(desc);
				return err;
			}
			if (func->builtin) {
				result = func->fun(vm, sc, v->children, v->count, func->sym);
			}
			else {
				result = eval_user_func(vm, sc, v->children, v->count, func);
			}
			return result;
		case LVAL_SYM:
			return scope_fetch_var(vm, sc, v->sym);
		case LVAL_ERR:
		case LVAL_FUN:
		case LVAL_NUM:
		case LVAL_BOOL:
		case LVAL_NULL:
		case LVAL_STR:
			return v;
	}

	return sexpr_err(vm, "Something hasn't been implemented yet");
}

void load_built_ins(scope *sc) {
   	scope_insert_var(sc, "car", sexpr_fun_builtin(&builtin_car, "car"));
	scope_insert_var(sc, "cdr", sexpr_fun_builtin(&builtin_cdr, "cdr"));
	scope_insert_var(sc, "cons", sexpr_fun_builtin(&builtin_cons, "cons"));
	scope_insert_var(sc, "list", sexpr_fun_builtin(&builtin_list, "list"));
	scope_insert_var(sc, "eq?", sexpr_fun_builtin(&builtin_eq, "eq?"));
	scope_insert_var(sc, "null?", sexpr_fun_builtin(&builtin_nullq, "null?"));
	scope_insert_var(sc, "pair?", sexpr_fun_builtin(&builtin_pairq, "pair?"));
	scope_insert_var(sc, "number?", sexpr_fun_builtin(&builtin_numberq, "number?"));
	scope_insert_var(sc, "eval", sexpr_fun_builtin(&builtin_eval, "eval"));
	scope_insert_var(sc, "+", sexpr_fun_builtin(&builtin_math_op, "+"));
	scope_insert_var(sc, "-", sexpr_fun_builtin(&builtin_math_op, "-"));
	scope_insert_var(sc, "*", sexpr_fun_builtin(&builtin_math_op, "*"));
	scope_insert_var(sc, "/", sexpr_fun_builtin(&builtin_math_op, "/"));
	scope_insert_var(sc, "%", sexpr_fun_builtin(&builtin_math_modulo, "%"));
	scope_insert_var(sc, "^", sexpr_fun_builtin(&builtin_math_op, "^"));
	scope_insert_var(sc, "=", sexpr_fun_builtin(&builtin_math_cmp, "="));
	scope_insert_var(sc, ">", sexpr_fun_builtin(&builtin_math_cmp, ">"));
	scope_insert_var(sc, ">=", sexpr_fun_builtin(&builtin_math_cmp, ">="));
	scope_insert_var(sc, "<", sexpr_fun_builtin(&builtin_math_cmp, "<"));
	scope_insert_var(sc, "<=", sexpr_fun_builtin(&builtin_math_cmp, "<="));
	scope_insert_var(sc, "not", sexpr_fun_builtin(&builtin_not, "not"));
	scope_insert_var(sc, "or", sexpr_fun_builtin(&builtin_or, "or"));
	scope_insert_var(sc, "and", sexpr_fun_builtin(&builtin_and, "and"));
	scope_insert_var(sc, "min", sexpr_fun_builtin(&builtin_min_op, "min"));
	scope_insert_var(sc, "max", sexpr_fun_builtin(&builtin_max_op, "max"));
	scope_insert_var(sc, "quit", sexpr_fun_builtin(&builtin_quit, "quit"));
	scope_insert_var(sc, "define", sexpr_fun_builtin(&define, "define"));
	scope_insert_var(sc, "quote", sexpr_fun_builtin(&quote_form, "quote"));
	scope_insert_var(sc, "lambda", sexpr_fun_builtin(&builtin_lambda, "lambda"));
	scope_insert_var(sc, "dump", sexpr_fun_builtin(&builtin_mem_dump, "dump"));
	scope_insert_var(sc, "cond", sexpr_fun_builtin(&builtin_cond, "cond"));
	scope_insert_var(sc, "if", sexpr_fun_builtin(&builtin_if, "if"));
	scope_insert_var(sc, "string?", sexpr_fun_builtin(&builtin_stringq, "string?"));
	scope_insert_var(sc, "string-length", sexpr_fun_builtin(&builtin_stringlen, "string-length"));
	scope_insert_var(sc, "string", sexpr_fun_builtin(&builtin_string, "string"));
	scope_insert_var(sc, "string-append", sexpr_fun_builtin(&builtin_stringappend, "string-append"));
	scope_insert_var(sc, "string-copy", sexpr_fun_builtin(&builtin_stringcopy, "string-copy"));
	scope_insert_var(sc, "load", sexpr_fun_builtin(&builtin_load, "load"));
}
