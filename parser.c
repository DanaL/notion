/* The parser accpets tokens one at a time via parser_feed_token()
	and builds up an expression one token at a time. Parser is probably
	too strong a word for it at the moment. Is only significant checking is
	making sure the open and close parantheses add up.

	It is not yet checking to see if a valid scheme expression is being made */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "sexpr.h"
#include "util.h"

sexpr *build_quote_form(vm_heap *vm, parser *p) {
	sexpr *sq = sexpr_list(vm);
	sexpr_append(sq, sexpr_sym(vm, "quote"));

	sexpr *quoted = get_next_expr(vm, p);
	if (quoted->type == LVAL_ERR)
		return quoted;

	sexpr_append(sq, quoted);

	return sq;
}

sexpr* sexpr_from_token(vm_heap *vm, parser *p, token *t) {
	sexpr *expr = NULL;

	switch (t->type) {
		case T_NUM:
			if (t->is_int)
				expr = sexpr_num(vm, NUM_TYPE_INT, t->i_num);
			else
				expr = sexpr_num(vm, NUM_TYPE_DEC, t->d_num);
			break;
		case T_SYM:
			expr = sexpr_sym(vm, t->val);
			break;
		case T_CONSTANT:
			if (strcmp("#t", t->val) == 0)
				expr = sexpr_bool(vm, 1);
			else if (strcmp("#f", t->val) == 0)
				expr = sexpr_bool(vm, 0);
			else
				expr = sexpr_err(vm, "Unknown constant.");
			break;
		case T_STR:
			expr = sexpr_str(vm, t->val);
			break;
		case T_ERR:
			expr = sexpr_err(vm, t->val);
			break;
		case T_SINGLE_QUOTE:
			return build_quote_form(vm, p);
		case T_LIST_START:
		case T_LIST_END:
		case T_NULL:
		case T_UNKNOWN:
		case T_COMMENT:
			expr = sexpr_err(vm, "Unexpected token.");
			break;
	}

	return expr;
}

parser* parser_new(tokenizer *tk) {
	parser *p = malloc(sizeof(parser));
	p->complete = 0;
	p->open_p = 0;
	p->closed_p = 0;
	p->curr = NULL;
	p->head = NULL;
	p->tk = tk;

	return p;
}

void parser_free(parser *p) {
	free(p);
}

sexpr *get_next_expr(vm_heap *vm, parser *p) {
	token *t = next_token(p->tk);

	while (t && t->type == T_COMMENT) {
		token_free(t);
		t = next_token(p->tk);
	}

	if (!t)
		return sexpr_null();

	if (t->type == T_LIST_START) {
		token_free(t);
		sexpr *list = sexpr_list(vm);

		t = next_token(p->tk);
		while (t && t->type != T_LIST_END) {
			if (t->type == T_LIST_START) {
				tokenizer_stash(p->tk, t);
				sexpr_append(list, get_next_expr(vm, p));
			}
			else if (t->type != T_COMMENT) {
				sexpr_append(list, sexpr_from_token(vm, p, t));
				token_free(t);
			}

			t = next_token(p->tk);
		}

		if (!t)
			return sexpr_err(vm, "Expected end of list.");
		else
			token_free(t);

		return list;
	}
	else {
		sexpr *e = sexpr_from_token(vm, p, t);
		token_free(t);
		return e;
	}

	return sexpr_err(vm, "Expected s-expression.");
}
