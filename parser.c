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

sexpr* sexpr_from_token(vm_heap *vm, token *t) {
	sexpr *expr = NULL;

	switch (t->type) {
		case T_NUM:
			expr = sexpr_num_s(vm, t->val);
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
		case T_LIST_START:
		case T_LIST_END:
		case T_NULL:
		case T_UNKNOWN:
		case T_COMMENT:
		case T_SINGLE_QUOTE:
			expr = sexpr_err(vm, "Unexpeted token.");
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
	tokenizer_free(p->tk);
	free(p);
}

void parser_clear(parser* p) {
	p->curr = NULL;
	p->head = NULL;
	p->complete = 0;
	p->open_p = 0;
	p->closed_p = 0;
}

sexpr *build_quote_form(vm_heap *vm, parser *p) {
	sexpr *sq = sexpr_list(vm);
	sexpr_list_insert(vm, sq, sexpr_sym(vm, "quote"));

	sexpr *quoted = get_next_expr(vm, p);
	if (quoted->type == LVAL_ERR)
		return quoted;

	sexpr_list_insert(vm, sq, quoted);

	return sq;
}

sexpr *build_list(vm_heap *vm, parser *p) {
	sexpr *list = sexpr_list(vm);

	token *t = next_token(p->tk);
	while (t && t->type != T_LIST_END) {
		if (t->type == T_LIST_START) {
			sexpr *l2 = build_list(vm, p);
			if (l2->type == LVAL_ERR)
				return l2;
			else
				sexpr_list_insert(vm, list, l2);
		}
		else if (t->type == T_SINGLE_QUOTE) {
			token_free(t);
			sexpr_list_insert(vm, list, build_quote_form(vm, p));
		}
		else {
			sexpr *item = sexpr_from_token(vm, t);
			sexpr_list_insert(vm, list, item);
		}

		token_free(t);
		t = next_token(p->tk);
	}

	if (!t)
		return sexpr_err(vm, "Expected end of list.");

	token_free(t);

	return list;
}

sexpr *get_next_expr(vm_heap *vm, parser *p) {
	token *t = next_token(p->tk);

	if (!t)
		return sexpr_null();
	else if (t->type == T_LIST_START) {
		sexpr *e = build_list(vm, p);
		if (e->type == LVAL_ERR)
			return e;

		return e;
	}
	else if (t->type == T_SINGLE_QUOTE) {
		token_free(t);
		return build_quote_form(vm, p);
	}
	else {
		sexpr *e = sexpr_from_token(vm, t);
		token_free(t);
		return e;
	}

	return sexpr_err(vm, "Expected s-expression.");
}
