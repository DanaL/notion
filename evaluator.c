#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "environment.h"
#include "sexpr.h"
#include "parser.h"
#include "util.h"

#define CLOSURE_TABLE_SIZE 89

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

	if (nodes[1]->type != LVAL_STR)
		return sexpr_err("Filename must be a string constant.");

	char buffer[1024];
	FILE *infile = fopen(nodes[1]->str, "r");

    parser *p = parser_create();
    tokenizer *t = tokenizer_create();

	if (!infile) {
		puts("File not found.");
	}
	else {
		sexpr *expr = NULL;

		while (fgets(buffer, 1024, infile)) {
            tokenizer_feed_line(t, buffer);
            token *nt;
            while (1) {
				nt = next_token(t);

                if (!nt)
                    break;
				else if (nt->type == T_COMMENT) {
					token_free(nt);
					continue;
				}
				else {
					parser_feed_token(p, nt);
					token_free(nt);
                    if (p->open_p == p->closed_p) {
                        expr = sexpr_copy(p->head);
						parser_clear(p);
                        sexpr *result = eval2(vm, env, expr);

						if (result->type != LVAL_NULL) {
							sexpr_pprint(result);
							putchar('\n');
						}
                    }
                }
            }
			if (nt)
				token_free(nt);
		}
	}

	tokenizer_free(t);
	parser_free(p);
	fclose(infile);

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

	return sexpr_bool(pq);
}

sexpr *builtin_not(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	if (count != 2)
		return sexpr_err("Just one parameter expected.");

	sexpr *v = eval2(vm, env, nodes[1]);
	if (v->type != LVAL_BOOL)
		return sexpr_err("Boolean value expected.");

	sexpr *result = sexpr_bool(!v->bool);

	return result;
}

/* I find Scheme's version of and pretty odd in how all non-booleans
	are considered true. */
sexpr* builtin_and(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(1);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			return cp;
		}

		if (cp->type == LVAL_BOOL && !cp->bool)
			return sexpr_bool(0);

		result = sexpr_copy(cp);
	}

	return result;
}

sexpr* builtin_or(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	/* and with no parameters returns true, apparently */
	if (count == 1)
		return sexpr_bool(0);

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			return cp;
		}

		if (cp->type != LVAL_BOOL || (cp->type == LVAL_BOOL && cp->bool))
			return cp;

		result = sexpr_copy(cp);
	}

	return result;
}

/* Converting integers into floats and then comparing them that way is
	probably deeply flawed, but the code sure is cleaner than trying to sort
	out if we are comparing ints to ints, floats to floats or a mix.

	It should be sufficiently fast and accurate for anything I might want to
	use notion for.
 */
sexpr* builtin_math_cmp(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	if (count != 3) {
		return sexpr_err("Just two parameters expected.");
	}

	sexpr *n0 = eval2(vm, env, nodes[1]);
	sexpr *n1 = eval2(vm, env, nodes[2]);

	if (n0->type != LVAL_NUM || n1->type != LVAL_NUM)
		return sexpr_err("Number expected.");

	int eq = 0;
	float f0 = n0->num_type == NUM_TYPE_INT ? n0->i_num : n0->d_num;
	float f1 = n1->num_type == NUM_TYPE_INT ? n1->i_num : n1->d_num;

	if (strcmp("=", op) == 0)
		eq = fabsf(f0 - f1) < 0.000000001;
	else if (strcmp("<", op) == 0)
		eq = f0 < f1;
	else if (strcmp(">", op) == 0)
		eq = f0 > f1;
	else if (strcmp(">=", op) == 0)
		eq = f0 > f1 || fabsf(f0 - f1) < 0.000000001;
	else if (strcmp("<=", op) == 0)
		eq = f0 < f1 || fabsf(f0 - f1) < 0.000000001;

	return eq ? sexpr_bool(1) : sexpr_bool(0);
}

sexpr* builtin_math_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	float result;
	sexpr *r;

	/* Unary subtraction */
	if (strcmp(op, "-") == 0 && count == 2) {
		sexpr *n = eval2(vm, env, nodes[1]);

		if (n->type != LVAL_NUM)
			r = sexpr_err("Expected number!");
		else {
			result = - (NUM_CONVERT(nodes[1]));
			r = sexpr_num(nodes[1]->num_type, result);
		}

		return r;
	}

	enum sexpr_num_type rt = NUM_TYPE_INT;
	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);

		if (n->type != LVAL_NUM) {
			return sexpr_err("Expected number!");
		}

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
			/* Let's make sure we're not trying to divide by zero */
			if (is_zero(n)) {
				return sexpr_err("Division by zero!");
			}
			result /= NUM_CONVERT(n);
		}
		else if (strcmp(op, "%") == 0) {
			if (n->num_type != NUM_TYPE_INT || nodes[1]->num_type != NUM_TYPE_INT) {
				return sexpr_err("Can only calculate the remainder for integers.");
			}
			else if (is_zero(n)) {
				return sexpr_err("Division by zero!");
			}
			result = nodes[1]->i_num % n->i_num;
		}
	}

	r = sexpr_num(rt, result);

	return r;
}

sexpr* builtin_min_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 2, "At least one parameter needed for min");

	float curr_max, x;

	enum sexpr_num_type rt = NUM_TYPE_INT;
	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);
		if (n->type != LVAL_NUM) {
			return sexpr_err("Expected number!");
		}

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

	return sexpr_num(rt, curr_max);
}

sexpr* builtin_max_op(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_MIN(count, 2, "At least one parameter needed for max");

	float curr_max, x;
	enum sexpr_num_type rt = NUM_TYPE_INT;

	for (int j = 1; j < count; j++) {
		sexpr *n = eval2(vm, env, nodes[j]);
		if (n->type != LVAL_NUM) {
			return sexpr_err("Expected number!");
		}

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

	return sexpr_num(rt, curr_max);
}

sexpr* builtin_list(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	sexpr* result = sexpr_list();

	for (int j = 1; j < count; j++) {
		sexpr *cp = eval2(vm, env, nodes[j]);
		if (cp->type == LVAL_ERR) {
			return cp;
		}

		sexpr_append(result, cp);
	}

	return result;
}

sexpr* builtin_cdr(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "cdr expects only one argument");

	sexpr *l = eval2(vm, env, nodes[1]);
	if (l->type != LVAL_LIST || l->count == 0) {
		return sexpr_err("cdr is defined only for non-empty lists.");
	}

	sexpr *result = sexpr_list();
	for (int j = 1; j < l->count; j++) {
		sexpr_list_insert(result, sexpr_copy(l->children[j]));
	}

	return result;
}

sexpr* builtin_car(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "car expects only one argument");

	sexpr *l = eval2(vm, env, nodes[1]);

	if (l->type != LVAL_LIST || l->count == 0) {
		return sexpr_err("car is defined only for non-empty lists.");
	}

	sexpr *result = sexpr_copy(l->children[0]);

	return result;
}

sexpr* builtin_cons(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "cons expects two aruments");

	sexpr *a2 = eval2(vm, env, nodes[2]);

	if (a2->type != LVAL_LIST) {
		return sexpr_err("The second argument of cons must be a list.");
	}

	sexpr *result = sexpr_list();
	sexpr_list_insert(result, eval2(vm, env, nodes[1]));

	for (int j = 0; j < a2->count; j++) {
		sexpr_list_insert(result, sexpr_copy(a2->children[j]));
	}

	return result;
}

sexpr* builtin_nullq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "null? expects just 1 argument");

	sexpr *a = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(a);

	sexpr *result = sexpr_bool(a->type == LVAL_LIST && a->count == 0 ? 1 :0);

	return result;
}

sexpr* builtin_numberq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "number? expects just 1 argument");

	sexpr *n = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(n);

	sexpr *result = sexpr_bool(n->type == LVAL_NUM);

	return result;
}

/* eq? as defined in the Little Schemer operates only on non-numeric atoms,
		but Scheme implementations I've seen accept broader inputs. I'm going to
		stick to the Little Schemer "standard" for now */
sexpr* builtin_eq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "eq? expects exactly 2 arguments.");

	sexpr *a = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(a);
	sexpr *b = eval2(vm, env, nodes[2]);
	if (b->type == LVAL_ERR) {
		return b;
	}

	sexpr *result = NULL;
	if (a->type == LVAL_BOOL && b->type == LVAL_BOOL && a->bool == b->bool)
		result = sexpr_bool(1);
	else if (a->type == LVAL_SYM && b->type == LVAL_SYM && strcmp(a->sym, b->sym) == 0)
		result = sexpr_bool(1);
	else if (a->type == LVAL_STR && b->type == LVAL_STR && strcmp(a->str, b->str) == 0)
		result = sexpr_bool(1);
	else if (a->type == LVAL_NUM && b->type == LVAL_NUM && a->num_type == b->num_type) {
		if (a->num_type == NUM_TYPE_INT && a->i_num == b->i_num)
			result = sexpr_bool(1);
		else if (a->num_type == NUM_TYPE_DEC && fabs(a->d_num - b->d_num) < 0.0000001)
			result = sexpr_bool(1);
		else
			result = sexpr_bool(0);
	}
	else
		result = sexpr_bool(0);

	return result;
}

sexpr* builtin_eval(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "eval expects just 1 argument");

	sexpr *f = eval2(vm, env, nodes[1]);

	if (IS_FUNC(f)) {
		sexpr *f2 = eval2(vm, env, f);
		f = f2;
	}

	return f;
}

sexpr* builtin_quit(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	return sexpr_err("<quit>");
}

sexpr* quote_form(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
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

sexpr* define_var(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op) {
	if (nodes[1]->type != LVAL_SYM)
		return sexpr_err("Expected variable name.");

	ASSERT_PRIMITIVE(sc, nodes[1]->sym);

	if (is_quoted_val(nodes[2]))
		scope_insert_var(vm, sc, nodes[1]->sym, sexpr_copy(nodes[2]->children[1]));
	else
		scope_insert_var(vm, sc, nodes[1]->sym, eval2(vm, sc, nodes[2]));

	return sexpr_null();
}

sexpr* build_func(vm_heap *vm, sexpr *header, sexpr *body, char *name) {
	/* Each parameter must be a symbol and be uniquely named
		Note to self: there can be zero params of course */
	sexpr *params = sexpr_list();
	for (int j = 1; j < header->count; j++) {
		if (header->children[j]->type != LVAL_SYM) {
			return sexpr_err("Paramter names must be symbols.");
		}
		sexpr_append(params, sexpr_copy(header->children[j]));
	}

	return sexpr_fun_user(params, sexpr_copy(body), name);
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
				return sexpr_err("Invalid boolean test.");
			}

			if (result->bool)
				return eval2(vm, env, cond->children[1]);
		}
		else
			return sexpr_err("Cond tests must be an expression.");
	}

	return sexpr_null();
}

sexpr* builtin_stringq(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String? takes just one paramter.");

	sexpr *s = eval2(vm, env, nodes[1]);
	sexpr *result = sexpr_bool(s->type == LVAL_STR ? 1 : 0);

	return result;
}

sexpr* builtin_stringlen(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String? takes just one paramter.");

	sexpr *s = eval2(vm, env, nodes[1]);
	if (s->type != LVAL_STR) {
		return sexpr_err("That was not a string.");
	}

	sexpr *result = sexpr_num(NUM_TYPE_INT, strlen(s->str));

	return result;
}

sexpr* builtin_string(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 2, "String takes just one parameter.");

	sexpr *src = eval2(vm, env, nodes[1]);
	if (src->type != LVAL_STR) {
		return sexpr_err("String takes a string type for its parameter.");
	}

	sexpr *result = sexpr_copy(src);

	return result;
}

sexpr* builtin_stringappend(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "String-append takse just two paramters");

	sexpr *s1 = eval2(vm, env, nodes[1]);
	ASSERT_NOT_ERR(s1);
	sexpr *s2 = eval2(vm, env, nodes[2]);
	if (s2->type == LVAL_ERR) {
		return s2;
	}

	sexpr *result;
	if (s1->type != LVAL_STR || s2->type != LVAL_STR) {
		result = sexpr_err("String-append requiers two strings.");
	}
	else {
		/* s1 and s2 are strings, but their values could be NULL */
		int len = 1 + (s1 ? strlen(s1->str) : 0);
		len += s2 ? strlen(s2->str) : 0;

		/* Competent C coders are probably recoiling in horror
			from this next bit... */
		result = sexpr_str(NULL);
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

	if (s1->type != LVAL_STR) {
		return sexpr_err("String copy only copies strings.");
	}

	/* Eval returns a copy of its input for atoms so we don't need to do
		anything else. */
	return s1;
}

sexpr* builtin_lambda(vm_heap *vm, scope *env, sexpr **nodes, int count, char *op) {
	ASSERT_PARAM_EQ(count, 3, "Invalid lambda definition.");

	/* So function evaluation excepts the paramter list for function
		calls to include the function name, so gotta add in a dummy
		value for lambdas */
	sexpr *params = sexpr_list();
	sexpr_append(params, sexpr_null());
	for (int j = 0; j < nodes[1]->count; j++)
		sexpr_append(params, sexpr_copy(nodes[1]->children[j]));
	sexpr *body = nodes[2];

	if (params->count == 0)
		return sexpr_err("Invalid definition.");

	sexpr *lambda = build_func(vm, params, body, "");

	return lambda;
}

sexpr* define_fun(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op)  {
	sexpr *header = nodes[1];
	sexpr *body = nodes[2];

	if (count < 3)
		return sexpr_err("Invalid definition.");

	ASSERT_PRIMITIVE(sc, header->children[0]->sym);
	char *fun_name = header->children[0]->sym;

	sexpr *fun = build_func(vm, header, body, fun_name);

	if (fun->type == LVAL_ERR)
		return fun;

	scope_insert_var(vm, sc, fun_name, fun);

	return sexpr_null();
}

sexpr* define(vm_heap *vm, scope *sc, sexpr **nodes, int count, char *op) {
	if (count != 3)
		return sexpr_err("Invalid definition.");

	if (nodes[1]->type == LVAL_LIST)
		return define_fun(vm, sc, nodes, count, op);
	else
		return define_var(vm, sc, nodes, count, op);
}

sexpr* resolve_symbol(scope *sc, sexpr *s) {
	sexpr *r = scope_fetch_var(sc, s->sym);

	if (r->type == LVAL_SYM) {
		sexpr *r2 = scope_fetch_var(sc, r->sym);
		/* If the symbol isn't found, just return the original result.
			(This is for cases like (define x 'aaa) where aaa is a meaningless
			symbol) */
		return r2->type == LVAL_ERR ? r : r2;
	}

	return r;
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
			var = scope_fetch_var(sc, operands[j + 1]->sym);
		else if (operands[j + 1]->type == LVAL_LIST)
			var = eval2(vm, sc, operands[j + 1]);
		else
			var = sexpr_copy(operands[j + 1]);

		if (var->type == LVAL_ERR)
			return var;

		printf("%s -> ", fun->params->children[j]->sym);
		sexpr_pprint(var);
		putchar('\n');
		scope_insert_var(vm, func_scope, fun->params->children[j]->sym, var);
	}

	sexpr_pprint(fun->body);
	putchar('\n');
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
				return sexpr_list();

			sexpr *func;
			if (v->children[0]->type == LVAL_LIST)
				func = eval2(vm, sc, v->children[0]);
			else if (v->children[0]->type == LVAL_SYM)
				func = scope_fetch_var(sc, v->children[0]->sym);
			else
				return sexpr_err("Expected function.");

			if (func->type != LVAL_FUN)
				return sexpr_err("Expected function.");

			if (func->builtin)
				result = func->fun(vm, sc, v->children, v->count, func->sym);
			else
				result = eval_user_func(vm, sc, v->children, v->count, func);

			return result;
		case LVAL_SYM:
			return resolve_symbol(sc, v);
		case LVAL_FUN:
			break;
		case LVAL_ERR:
			return v;
		case LVAL_NUM:
		case LVAL_BOOL:
		case LVAL_NULL:
		case LVAL_STR:
			return sexpr_copy(v);
			break;
	}

	return sexpr_err("Something hasn't been implemented yet");
}

void load_built_ins(vm_heap *vm, scope *sc) {
   	scope_insert_var(vm, sc, "car", sexpr_fun_builtin(&builtin_car, "car"));
	scope_insert_var(vm, sc, "cdr", sexpr_fun_builtin(&builtin_cdr, "cdr"));
	scope_insert_var(vm, sc, "cons", sexpr_fun_builtin(&builtin_cons, "cons"));
	scope_insert_var(vm, sc, "list", sexpr_fun_builtin(&builtin_list, "list"));
	scope_insert_var(vm, sc, "eq?", sexpr_fun_builtin(&builtin_eq, "eq?"));
	scope_insert_var(vm, sc, "null?", sexpr_fun_builtin(&builtin_nullq, "null?"));
	scope_insert_var(vm, sc, "pair?", sexpr_fun_builtin(&builtin_pairq, "pair?"));
	scope_insert_var(vm, sc, "number?", sexpr_fun_builtin(&builtin_numberq, "number?"));
	scope_insert_var(vm, sc, "eval", sexpr_fun_builtin(&builtin_eval, "eval"));
	scope_insert_var(vm, sc, "+", sexpr_fun_builtin(&builtin_math_op, "+"));
	scope_insert_var(vm, sc, "-", sexpr_fun_builtin(&builtin_math_op, "-"));
	scope_insert_var(vm, sc, "*", sexpr_fun_builtin(&builtin_math_op, "*"));
	scope_insert_var(vm, sc, "/", sexpr_fun_builtin(&builtin_math_op, "/"));
	scope_insert_var(vm, sc, "%", sexpr_fun_builtin(&builtin_math_op, "%"));
	scope_insert_var(vm, sc, "=", sexpr_fun_builtin(&builtin_math_cmp, "="));
	scope_insert_var(vm, sc, ">", sexpr_fun_builtin(&builtin_math_cmp, ">"));
	scope_insert_var(vm, sc, ">=", sexpr_fun_builtin(&builtin_math_cmp, ">="));
	scope_insert_var(vm, sc, "<", sexpr_fun_builtin(&builtin_math_cmp, "<"));
	scope_insert_var(vm, sc, "<=", sexpr_fun_builtin(&builtin_math_cmp, "<="));
	scope_insert_var(vm, sc, "not", sexpr_fun_builtin(&builtin_not, "not"));
	scope_insert_var(vm, sc, "or", sexpr_fun_builtin(&builtin_or, "or"));
	scope_insert_var(vm, sc, "and", sexpr_fun_builtin(&builtin_and, "and"));
	scope_insert_var(vm, sc, "min", sexpr_fun_builtin(&builtin_min_op, "min"));
	scope_insert_var(vm, sc, "max", sexpr_fun_builtin(&builtin_max_op, "max"));
	scope_insert_var(vm, sc, "quit", sexpr_fun_builtin(&builtin_quit, "quit"));
	scope_insert_var(vm, sc, "define", sexpr_fun_builtin(&define, "define"));
	scope_insert_var(vm, sc, "quote", sexpr_fun_builtin(&quote_form, "quote"));
	scope_insert_var(vm, sc, "lambda", sexpr_fun_builtin(&builtin_lambda, "lambda"));
	scope_insert_var(vm, sc, "dump", sexpr_fun_builtin(&builtin_mem_dump, "dump"));
	scope_insert_var(vm, sc, "cond", sexpr_fun_builtin(&builtin_cond, "cond"));
	scope_insert_var(vm, sc, "string?", sexpr_fun_builtin(&builtin_stringq, "string?"));
	scope_insert_var(vm, sc, "string-length", sexpr_fun_builtin(&builtin_stringlen, "string-length"));
	scope_insert_var(vm, sc, "string", sexpr_fun_builtin(&builtin_string, "string"));
	scope_insert_var(vm, sc, "string-append", sexpr_fun_builtin(&builtin_stringappend, "string-append"));
	scope_insert_var(vm, sc, "string-copy", sexpr_fun_builtin(&builtin_stringcopy, "string-copy"));
	scope_insert_var(vm, sc, "load", sexpr_fun_builtin(&builtin_load, "load"));
}
