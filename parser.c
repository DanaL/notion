#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "sexpr.h"

token* token_create(enum token_type type) {
	token *t = malloc(sizeof(token));
	t->type = type;
	t->val = NULL;

	return t;
}

void token_free(token *t) {
	if (t->val != NULL)
		free(t->val);

	free(t);
}

int skip_whitespace(char *s, int x) {
	int j, end = strlen(s);
	for (j = x; j < end && s[j] == ' '; j++);

	return j;
}

void print_token(token *t) {
	switch (t->type) {
		case T_LIST_START:
			puts("(");
			break;
		case T_LIST_END:
			puts(")");
			break;
		case T_SYM:
			printf("Symbol: %s\n", t->val);
			break;
		case T_NUM:
			printf("Number: %s\n", t->val);
			break;
		case T_STR:
			printf("String: %s\n", t->val);
			break;
		case T_NULL:
			puts("Null token");
			break;
		case T_UNKNOWN:
			printf("Unknown token: %s\n", t->val);
			break;
		case T_CONSTANT:
			printf("Constant: %s\n", t->val);
			break;
		case T_SINGLE_QUOTE:
			printf("Single quote\n");
			break;
	}
}

int is_valid_in_symbol(char c)
{
	if (isalpha(c) || isdigit(c))
		return 1;

	switch (c) {
		case '@':
		case '#':
		case '$':
		case '&':
		case ':':
		case '|':
		case '?':
		case '.':
		case '_':
		case '-':
			return 1;
	}

	return 0;
}

token* next_token(char *s, int *start) {
	token *t = NULL;
	int len, x = 0;

	*start = skip_whitespace(s, *start);

	if (s[*start] == '\0')
		return token_create(T_NULL);

	if (s[*start] == '+' || s[*start] == '*' || s[*start] == '/' || s[*start] == '%') {
		t = token_create(T_SYM);
		x = *start + 1;
	}
	else if (s[*start] == '\'') {
		t = token_create(T_SINGLE_QUOTE);
		x = *start + 1;
	}
	else if(s[*start] == '(') {
		t = token_create(T_LIST_START);
		x = *start + 1;
	}
	else if(s[*start] == ')') {
		t = token_create(T_LIST_END);
		x = *start + 1;
	}
	else if (s[*start] == '-' || isdigit(s[*start])) {
		int dot_found = 0;
		/* It's either a negative number or a subtraction function */
		x = *start + 1;

		/* This is terrible and I should turn this whole function into a state machine */
		while (s[x] != '\0' && (isdigit(s[x]) || (s[x] == '.' && !dot_found))) {
			if (s[x] == '.')
				dot_found = 1;
			++x;
		}

		if (s[*start] == '-' && x - *start == 1)
			t = token_create(T_SYM);
		else
			t = token_create(T_NUM);
	}
	else if (isalpha(s[*start])) {
		t = token_create(T_SYM);
		x = *start + 1;
		while (s[x] != '\0' && is_valid_in_symbol(s[x]))
			++x;
	}
	else if (s[*start] == '#') {
		t = token_create(T_CONSTANT);
		x = *start + 1;
		while (s[x] != '\0' && isalpha(s[x]))
			++x;
	}
	else {
		t = token_create(T_UNKNOWN);
		x = *start + 1;
	}

	len = x - *start + 1;
	t->val = malloc(len * sizeof(char));
	memcpy(t->val, &s[*start], len - 1);
	t->val[len - 1] = '\0';
	(*start) = x;

	return t;
}

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
		case T_LIST_START:
		case T_LIST_END:
		case T_STR:
		case T_NULL:
		case T_UNKNOWN:
			printf("\"%s\"\n", t->val);
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
			else {
				child = sexpr_from_token(nt);
			}

			if (child->type == LVAL_ERR) {
				token_free(nt);
				sexpr_free(expr);
				return child;
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
	else
		expr = sexpr_from_token(nt);

	token_free(nt);

	return expr;
}
