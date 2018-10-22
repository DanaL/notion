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

parser* parser_create(void) {
	parser *p = malloc(sizeof(parser));
	p->complete = 0;
	p->open_p = 0;
	p->closed_p = 0;
	p->curr = NULL;
	p->head = NULL;

	return p;
}

void parser_free(parser *p) {
	free(p);
}

void parser_clear(parser* p) {
	p->curr = NULL;
	p->head = NULL;
	p->complete = 0;
	p->open_p = 0;
	p->closed_p = 0;
}

void parser_feed_token(vm_heap *vm, parser* p, token* t) {
	/* If we have a open parenthesis, start a new list and make it our current
		head. If head is an existing list, we append the new list to its lists
		if children. */
	if (t->type == T_LIST_START) {
		sexpr *list = sexpr_list(vm);

		/* Are we at the start of a brand new expression? */
		if (!p->head)
			p->head = list;

		list->parent = p->curr;

		if (p->curr)
			sexpr_append(p->curr, list);

		p->curr = list;
		p->open_p++;
	}
	else if (t->type == T_SINGLE_QUOTE) {
		/* This is the case of: (car '(1 2 3)), which I want to expand to:
			(car (quote (1 2 3)))

			So start a new list, add a quote symbol to it and make that the
			new curr pointer.
		 */
		sexpr *quote = sexpr_list(vm);

		/* This is a bit kludgy :( If this, the single quoted item we are
			expanding in a (quote ...) expression contains a list, when we
			hit its ending paranthesis, we will need to pop out two levels
			instead of one, so track that with the sq_list field */
		quote->sq_list = 1;
		sexpr_append(quote, sexpr_sym(vm, "quote"));

		/* I don't really need to increment the counts but I know sometime in
			the future I'll be debugging and manually counting them and panic
			when they don't match what the program has */
		p->open_p++;
		p->closed_p++;

		quote->parent = p->curr;
		if (p->curr)
			sexpr_append(p->curr, quote);
		else {
			/* Case where a single quoted value is by itself, mark as complete
				ie., 'foo */
			p->head = quote;
			p->complete = 1;
		}

		p->curr = quote;
	}
	else if (t->type == T_LIST_END) {
		/* the parent appears to be NULL, it's because an extra close
			parenthesis was typed. Ie,  (+ (list 1 2 3)) (list 3 2 1))
			So flag that as an error condition. */
		if (!p->curr) {
			parser_clear(p);
			p->head = sexpr_err(vm, "Unexpected end of list. Too many )s?");
			p->complete = 1;
			return;
		}

		/* Pop up the chain to the parent, or the grandparent if we are
		 	finishing a list that was single quoted, ie., '(1 2 3) */
		p->curr = p->curr->parent;
		if (p->curr && p->curr->sq_list)
			p->curr = p->curr->parent;

		p->closed_p++;
		if (p->open_p == p->closed_p)
			p->complete = 1;
	}
	else {
		sexpr *e = sexpr_from_token(vm, t);

		if (e->type == LVAL_ERR) {
			parser_clear(p);
			p->head = e;
			p->complete = 1;
			return;
		}

		/* Okay, if we have an atom and the head is empty, our s-expression
		 	is just a single atom, otherwise we want to append it to curr */
		if (!p->head && IS_ATOM(e)) {
			p->head = e;
			p->complete = 1;

			return;
		}
		else {
			sexpr_append(p->curr, e);

			/* I hate Scheme's '(1 2 3) and 'foo special form :/
				If we are an atom and our parent was marked as a single-quoted
				list, we want to set curr equal to its parent (because with the
				faked list for the special form, we never get the end-of-list
				token) */
			if (p->curr && p->curr->sq_list) {
				p->curr = p->curr->parent;
			}
		}
	}
}
