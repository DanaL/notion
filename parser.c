#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

token* token_create(enum token_type type, char* sym) {
	token *t = malloc(sizeof(token));
	t->type = type;

	if (sym != NULL) {
		t->val = malloc(strlen(sym) + 1);
		strcpy(t->val, sym);
	}
	else
		t->val = NULL;

	return t;
}

void token_free(token *t) {
	if (t->val != NULL)
		free(t->val);

	free(t);
}

lval* lval_num(lval* j) {
	lval *v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num_type = j->num_type;

	if (j->num_type == NUM_TYPE_INT) {
		v->n.i_num = j->n.i_num;
	}
	else
		v->n.d_num = j->n.d_num;

	return v;
}

lval* lval_num_s(char *s) {
	lval *v = malloc(sizeof(lval));
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

lval* lval_err(char *s) {
	lval *v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(s) + 1);
	strcpy(v->err, s);

	return v;
}

lval* lval_sym(char *s) {
	lval *v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);

	return v;
}

lval* lval_sexpr(void) {
	lval *v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->children = NULL;
	
	return v;
}

lval* lval_list(void) {
	lval *v = malloc(sizeof(lval));
	v->type = LVAL_LIST;
	v->count = 0;
	v->children = NULL;

	return v;
}

void lval_free(lval *v) {
	switch (v->type) {
		case LVAL_NUM:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_SEXPR:
		case LVAL_LIST:
			for (int j = 0; j < v->count; j++)
				lval_free(v->children[j]);
			break;
	}

	free(v);
}

lval* lval_copy_atom(lval* src) {
	lval* dst;

	if (src->type == LVAL_NUM) {
		dst = lval_num(src);
		return dst;		
	}
	else if (src->type == LVAL_SYM) {
		dst = lval_sym(src->sym);
		return dst;
	}

	return lval_err("Can only copy atoms.");
}

void lval_copy_list(lval* dst, lval* src) {
	for (int j = 0; j < src->count; j++) {
		if (src->children[j]->type == LVAL_NUM || src->children[j]->type == LVAL_SYM) {
			lval *cp = lval_copy_atom(src->children[j]);
			lval_append(dst, cp);
		}
		else if (src->children[j]->type == LVAL_LIST) {
			lval *cp = lval_list();
			lval_copy_list(cp, src->children[j]);
			lval_append(dst, cp);
		}
	}
}

lval* lval_append(lval *v, lval *next) {
	v->count++;
	v->children = realloc(v->children, sizeof(lval*) * v->count);	
	v->children[v->count - 1] = next;

	return v;
}

int skip_whitespace(char *s, int x) {
	int j, end = strlen(s);
	for (j = x; j < end && s[j] == ' '; j++);

	return j;
}

void print_token(token *t) {
	switch (t->type) {
		case T_SEXPR_START:
			puts("(");
			break;
		case T_SEXPR_END:
			puts(")");
			break;
		case T_OP:
			printf("Operator: %s\n", t->val);
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
		return token_create(T_NULL, NULL);
	
	if (s[*start] == '+' || s[*start] == '*' || s[*start] == '/' || s[*start] == '%') {
		t = token_create(T_OP, NULL);
		x = *start + 1;
	}
	else if(s[*start] == '(') {
		t = token_create(T_SEXPR_START, NULL);
		x = *start + 1;
	}
	else if(s[*start] == ')') {
		t = token_create(T_SEXPR_END, NULL);
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
			t = token_create(T_OP, NULL);
		else
			t = token_create(T_NUM, NULL);
	}
	else if (isalpha(s[*start])) {
		t = token_create(T_STR, NULL);
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

int is_built_in(char *s) {
	int result = 0;
	char *ls = malloc(strlen(s) + 1);
	for (int j = 0; j < strlen(s); j++) {
		ls[j] = tolower(s[j]);
	}
	ls[strlen(s)] = '\0';

	if (strcmp(ls, "max") == 0)
		result = 1;
	else if (strcmp(ls, "min") == 0)
		result = 1;
	else if (strcmp(ls, "list") == 0)
		result = 1;

	free(ls);

	return result;
}

/* Parsing an s-expression is a simple state machine:

	We start at the beginning of an expression and (currently) we expect an operator. 
	Eventually we'll allow other s-expressions (+ (* 6 8) 4 -1) and user-defined functions. 

	So:
		s-expr 		-> operator OR number
		operator 	-> number OR s-expr
		number 		-> number OR end-of-s-expr OR s-expr

	Eventually we'll have nested s-exprs and empty s-exprs ()
*/
lval* parse_sexpr(char *s, int *curr) {
	lval *head = lval_sexpr();
	enum token_type curr_type = T_SEXPR_START;

	while (curr_type != T_SEXPR_END) {
		token *nt = next_token(s, curr);
	
		switch (curr_type) {
			case T_SEXPR_START:
				if (nt->type == T_OP || is_built_in(nt->val)) {
					lval *op = lval_sym(nt->val);
					lval_append(head, op);
					curr_type = T_OP;
				}
				else if (nt->type == T_NUM) {
					lval *num = lval_num_s(nt->val);
					lval_append(head, num);
					curr_type = T_NUM;
				}
				else {
					token_free(nt);
					lval_free(head);
					return lval_err("Expected operator.");
				}
				break;
			case T_OP:
				if (nt->type == T_NUM) {
					lval *num = lval_num_s(nt->val);
					lval_append(head, num);
					curr_type = T_NUM;
				}
				else if (nt->type == T_SEXPR_START) {
					lval *sub_expr = parse_sexpr(s, curr);
					if (sub_expr->type == LVAL_ERR) {
						lval_free(head);
						token_free(nt);
						return sub_expr;
					}

					lval_append(head, sub_expr);
					curr_type = T_NUM;
				}
				else {
					token_free(nt);
					lval_free(head);
					return lval_err("Expected atom or start of s-expr.");
				}
				break;
			case T_NUM:
				if (nt->type == T_NUM) {
					lval *num = lval_num_s(nt->val);
					lval_append(head, num);
					curr_type = T_NUM;
				}
				else if (nt->type == T_SEXPR_START) {
					lval *sub_expr = parse_sexpr(s, curr);
					if (sub_expr->type == LVAL_ERR) {
						lval_free(head);
						token_free(nt);
						return sub_expr;
					}

					lval_append(head, sub_expr);
					curr_type = T_NUM;
				}
				else if (nt->type == T_SEXPR_END) {
					curr_type = T_SEXPR_END;
				}
				else {
					token_free(nt);
					lval_free(head);
					return lval_err("Expected end of s-expr.");
				}
				break;
			case T_STR:
			case T_NULL:
			case T_SEXPR_END:
				token_free(nt);
				lval_free(head);
				return lval_err("Unexpeted token");
		}

		token_free(nt);
	}

	return head;
}

lval* parse(char *s) {
	int j = 0;
	
	token *nt = next_token(s, &j);
	if (nt->type != T_SEXPR_START) {
		token_free(nt);
		return lval_err("Expected start of s-expr!");
	}

	lval *head = parse_sexpr(s, &j);
	
	return head;
}

void print_padding(int depth) {
	for (int j = 0; j < depth * 4; j++) putchar(' ');
}

void lval_pprint(lval *v, int depth) {
	switch (v->type) {
		case LVAL_ERR:
			printf("Error: %s\n", v->err);
			break;
		case LVAL_SEXPR:
			putchar('\n');
			print_padding(depth);
			putchar('(');

			int j = 0;
			while (j < v->count) {
				lval_pprint(v->children[j++], depth + 1);
				if (j < v->count)
					putchar(' ');
			}
			putchar('\n');
			print_padding(depth);
			puts(")");

			break;
		case LVAL_LIST:
			print_padding(depth);
			putchar('(');

			for (int j = 0; j < v->count; j++) {
				lval_pprint(v->children[j], depth + 1);
				if (j < v->count)
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
