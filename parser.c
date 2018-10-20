#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "sexpr.h"
#include "util.h"

sexpr* sexpr_from_token(token *t) {
	sexpr *expr = NULL;

	switch (t->type) {
		case T_NUM:
			expr = sexpr_num_s(t->val);
			break;
		case T_SYM:
			expr = sexpr_sym(t->val);
			break;
		case T_CONSTANT:
			if (strcmp("#t", t->val) == 0)
				expr = sexpr_bool(1);
			else if (strcmp("#f", t->val) == 0)
				expr = sexpr_bool(0);
			else
				expr = sexpr_err("Unknown constant.");
			break;
		case T_STR:
			expr = sexpr_str(t->val);
			break;
		case T_ERR:
			expr = sexpr_err(t->val);
			break;
		case T_LIST_START:
		case T_LIST_END:
		case T_NULL:
		case T_UNKNOWN:
		case T_SINGLE_QUOTE:
			expr = sexpr_err("Unexpeted token.");
			break;
	}

	return expr;
}

void dump_tokens(char *s) {
		int c = 0;

		token *nt = next_token(s, &c);
		while (nt && nt->type != T_NULL) {
			print_token(nt);
			nt = next_token(s, &c);
		}
}

/* Either return an atom, or if we find the start of a list,
	keep pulling the next token until we hit the end of the list or
	run out of tokens (which is an error condition).

	If we encounter a nested list, it's recursion time! */
sexpr* parse(char *s, int *curr) {
	sexpr *expr = NULL;

	token *nt = next_token(s, curr);
	if (nt->type == T_LIST_START) {
		expr = sexpr_list();

		token_free(nt);
		nt = next_token(s, curr);
		while (nt->type != T_LIST_END) {
			sexpr *child = NULL;
			/* Single quote form, so I want to transform, say, 'Q into
				(quote Q) or '(1 2 3) into (quote (1 2 3)) */
			if (nt->type == T_SINGLE_QUOTE) {
				child = sexpr_list();
				sexpr_append(child, sexpr_sym("quote"));
				sexpr_append(child, parse(s, curr));
			}
			else if (nt->type == T_LIST_START) {
				(*curr)--; /* Back up the token otherwise we skip past the start of the list in the recursive call */
				child = parse(s, curr);
			}
			else if (nt->type == T_ERR) {
				sexpr_free(expr);
				expr = sexpr_from_token(nt);
				token_free(nt);
				return expr;
			}
			else if (nt->type == T_NULL) {
				sexpr_free(expr);
				token_free(nt);
				return sexpr_err("Unterminated s-expr. Get your parantheses in order!");
			}
			else {
				child = sexpr_from_token(nt);
			}

			sexpr_append(expr, child);
			token_free(nt);
			nt = next_token(s, curr);
		}
	}
	else if (nt->type == T_SINGLE_QUOTE) {
		expr = sexpr_list();
		sexpr_append(expr, sexpr_sym("quote"));
		sexpr_append(expr, parse(s, curr));
	}
	else if (nt->type == T_ERR) {
		if (expr)
			sexpr_free(expr);
		expr = sexpr_from_token(nt);
		token_free(nt);

		return expr;
	}
	else {
		expr = sexpr_from_token(nt);
	}

	token_free(nt);

	return expr;
}
