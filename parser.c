#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

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

sexpr* sexpr_num(sexpr* j) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;
	v->num_type = j->num_type;

	if (j->num_type == NUM_TYPE_INT) {
		v->n.i_num = j->n.i_num;
	}
	else
		v->n.d_num = j->n.d_num;

	return v;
}

sexpr* sexpr_num_s(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_NUM;

	if (strchr(s, '.')) {
		float f = strtof(s, NULL);
		v->num_type = NUM_TYPE_DEC;
		v->n.d_num = f;
	}
	else {
		long i = strtol(s, NULL, 10);
		v->num_type = NUM_TYPE_INT;
		v->n.i_num = i;
	}

	return v;
}

sexpr* sexpr_err(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(s) + 1);
	strcpy(v->err, s);

	return v;
}

sexpr* sexpr_sym(char *s) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);

	return v;
}

sexpr* sexpr_list(void) {
	sexpr *v = malloc(sizeof(sexpr));
	v->type = LVAL_LIST;
	v->count = 0;
	v->children = NULL;

	return v;
}

void sexpr_free(sexpr *v) {
	switch (v->type) {
		case LVAL_NUM:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_LIST:
			for (int j = 0; j < v->count; j++)
				sexpr_free(v->children[j]);
			break;
	}

	free(v);
}

sexpr* sexpr_copy_atom(sexpr* src) {
	sexpr* dst;

	if (src->type == LVAL_NUM) {
		dst = sexpr_num(src);
		return dst;		
	}
	else if (src->type == LVAL_SYM) {
		dst = sexpr_sym(src->sym);
		return dst;
	}

	return sexpr_err("Can only copy atoms.");
}

sexpr* sexpr_copy_list(sexpr* src) {
	sexpr *dst = sexpr_list();

	for (int j = 0; j < src->count; j++) {
		if (IS_ATOM(src->children[j])) {
			sexpr *cp = sexpr_copy_atom(src->children[j]);
			sexpr_append(dst, cp);
		}
		else if (src->children[j]->type == LVAL_LIST) {
			sexpr *cp = sexpr_list();
			sexpr_append(dst, sexpr_copy_list(src->children[j]));
		}
	}

	return dst;
}

sexpr* sexpr_copy(sexpr* src) {
	if (IS_ATOM(src))
		return sexpr_copy_atom(src);

	return sexpr_copy_list(src);
}

void sexpr_append(sexpr *v, sexpr *next) {
	v->count++;
	v->children = realloc(v->children, sizeof(sexpr*) * v->count);	
	v->children[v->count - 1] = next;
}

void sexpr_list_insert(sexpr *dst, sexpr *item) {
	sexpr* cp = sexpr_copy(item);
	sexpr_append(dst, cp);
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
	}
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
		/*This is terrible and I should turn this whole function into a state machine */
		while (s[x] != '\0' && (isdigit(s[x]) || (s[x] == '.' && !dot_found))) {
			if (s[x] == '.')
				dot_found = 1;
			++x;
		}

		if (s[*start] == '-' && x - *start == 1)
			t = token_create(T_NUM);
		else
			t = token_create(T_NUM);
	}
	else if (isalpha(s[*start])) {
		t = token_create(T_SYM);
		x = *start + 1;
		while (s[x] != '\0' && (isalpha(s[x]) || isdigit(s[x])))
			++x;
	}

	len = x - *start + 1;
	t->val = malloc(len * sizeof(char));
	memcpy(t->val, &s[*start], len - 1);
	t->val[len - 1] = '\0';
	(*start) = x;

	return t;
}

/* Parsing an s-expression is a simple state machine:

	We start at the beginning of an expression and (currently) we expect an operator. 
	Eventually we'll allow other s-expressions (+ (* 6 8) 4 -1) and user-defined functions. 

	An sexpr is either an atom (number, symbol) or a list (which contains numbers, symbols, or other lists)
	So:
		atomic type -> return it
		start of list -> return sexpr with list of its items

*/
sexpr* sexpr_from_token(token *t) {
	sexpr *expr = NULL;

	switch (t->type) {
		case T_NUM:
			expr = sexpr_num_s(t->val);
			break;
		case T_SYM:
			expr = sexpr_sym(t->val);
			break;
		case T_LIST_START:
		case T_LIST_END:
		case T_STR:
		case T_NULL:
			expr = sexpr_err("Unexpeted token.");
			break;
	}

	return expr;
}

sexpr* parse(char *s, int *curr) {
	sexpr *expr = NULL;

	token *nt = next_token(s, curr);
	if (nt->type == T_LIST_START) {
		expr = sexpr_list();
		
		token_free(nt);
		nt = next_token(s, curr);
		while (nt->type != T_LIST_END) {
			sexpr *child = NULL;
			if (nt->type == T_LIST_START) {
				(*curr)--; /* Back up the token otherwise we skip past the start of the list in the recursive call */
				child = parse(s, curr);
			}
			else
				child = sexpr_from_token(nt);

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
	else
		expr = sexpr_from_token(nt);

	token_free(nt);
	
	return expr;
}

void print_padding(int depth) {
	for (int j = 0; j < depth * 4; j++) putchar(' ');
}

void sexpr_pprint(sexpr *v, int depth) {
	switch (v->type) {
		case LVAL_ERR:
			printf("Error: %s\n", v->err);
			break;
		case LVAL_LIST:
			putchar('\n');
			print_padding(depth);
			putchar('(');

			for (int j = 0; j < v->count; j++) {
				sexpr_pprint(v->children[j], depth + 1);
				if (j < v->count - 1)
					putchar(' ');
			}
			puts(")");
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_NUM:
			if (v->num_type == NUM_TYPE_INT)
				printf("%li", v->n.i_num);
			else
				printf("%f", v->n.d_num);
			break;
	}
}
