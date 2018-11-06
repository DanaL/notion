#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "util.h"

token* token_new(enum token_type type) {
	token *t = malloc(sizeof(token));
	t->type = type;
	t->val = NULL;
	t->next = NULL;

	return t;
}

void token_free(token *t) {
	if (!t)
		return;

    if (t->val != NULL)
		free(t->val);

	free(t);
}

void print_token(token *t) {
	switch (t->type) {
		case T_ERR:
			printf("Error: %s\n", t->val);
			break;
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
        case T_COMMENT:
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
		case '+':
		case '*':
		case '!':
			return 1;
	}

	return 0;
}

tokenizer* tokenizer_new(void) {
    tokenizer *t = malloc(sizeof(tokenizer));
    t->curr_line = NULL;
    t->pos = 0;
	t->file = NULL;
	t->stashed = NULL;

    return t;
}

void tokenizer_free(tokenizer* tk) {
	if (tk->file)
		fclose(tk->file);

	/* There shouldn't be any leftover tokens on the stash, but just in case */
	token *t;
	while (tk->stashed) {
		t = tk->stashed;
		tk->stashed = tk->stashed->next;
		token_free(t);
	}

    free(tk);
}

void tokenizer_feed_line(tokenizer* t, char* line) {
    if (t->curr_line)
        free(t->curr_line);

    t->curr_line = line ? n_strcpy(t->curr_line, line) : NULL;
    t->pos = 0;
}

int skip_whitespace(char *line, int pos) {
    int curr = pos;

    while (line && (line[curr] == ' ' || line[curr] == '\n'))
		++curr;

	return curr;
}

int is_number_token(token *tk) {
	char *p;

	long a =strtol(tk->val, &p, 10);
	if (strcmp(p, "") == 0) {
		tk->is_int = 1;
		tk->i_num = a;
		return 1;
	}

	double b = strtod(tk->val, &p);
	if (strcmp(p, "") == 0) {
		tk->is_int = 0;
		tk->d_num = b;
		return 1;
	}

	return 0;
}

token* parse_str_token(char *s, int *start) {
	token *tk = NULL;
	int len, x = *start + 1;

	while (s[x] != '\0' && s[x] != '"') {
		if (s[x] == '\\') {
			// The only escape characters I'm going to escape for now
			switch (s[x+1]) {
				case '\\':
				case '"':
					++x;
					break;
					++x;
					break;
				default:
					tk = token_new(T_ERR);
					goto err;
			}
 		}
		++x;
	}
	++x;

err:
	if (tk && tk->type == T_ERR) {
		tk->val = NULL;
		tk->val = n_strcpy(tk->val, "Invalid escape character.");
		return tk;
	}

	tk = token_new(T_STR);
	len = x - *start - 2; // Skip the quotes
	tk->val = malloc(1 + len * sizeof(char));

	// Copy string, skipping backslashes
	int j, k;
	for (j = *start + 1, k = 0; k < len; j++) {
		if (s[j] != '\\' || (s[j] == '\\' && s[j+1] == '\\'))
			tk->val[k++] = s[j];
		else
			--len;
	}
	tk->val[len] = '\0';

	(*start) = x;

	return tk;
}

char* trim_line(char *buffer) {
	int c = 0;

	while (isspace(buffer[c]) && buffer[c] != '\0')
		++c;

	int e = strlen(buffer) - 1;
	while (e > 0 && isspace(buffer[e]))
		--e;

	int diff = e - c + 1;
	char *line = malloc(diff + 1);
	memcpy(line, &buffer[c], diff );
	line[diff] = '\0';

	return line;
}

char* fetch_next_line(tokenizer *t) {
	char buffer[1024];
	buffer[0] = '\0';
	char *line = NULL;

	while (1) {
		if (!fgets(buffer, 1024, t->file)) {
			fclose(t->file);
			break;
		}
		line = trim_line(buffer);
		if (strlen(line) > 0)
			break;
		free(line);
	}

	return line;
}

token* next_in_line(tokenizer *tk) {
	token *t;
    tk->pos = skip_whitespace(tk->curr_line, tk->pos);
    char *s = tk->curr_line;
    int x = tk->pos;

	if (s[tk->pos] == '\0') {
		free(tk->curr_line);
		tk->curr_line = NULL;
		tk->pos = 0;

		return NULL;
	}

	if (tk->curr_line[tk->pos] == '(') {
        t = token_new(T_LIST_START);
        t->val =  n_strcpy(t->val, "(");
        tk->pos++;

        return t;
    }

    if (s[tk->pos] == '+' || s[tk->pos] == '*' || s[tk->pos] == '/' || s[tk->pos] == '%'
			|| s[tk->pos] == '=' || s[tk->pos] == '^'
			|| (s[tk->pos] == '-' && s[tk->pos] == ' ')) {
		t = token_new(T_SYM);
		x = tk->pos + 1;
	}
	else if (s[tk->pos] == '\\') {
		t = token_new(T_ERR);
		t->val = NULL;
		t->val = n_strcpy(t->val, "Invalid token.");
		return t;
	}
	else if (s[tk->pos] == '"') {
		t = parse_str_token(s, &(tk->pos));
		return t;
	}
	else if (s[tk->pos] == '<' || s[tk->pos] == '>') {
		t = token_new(T_SYM);
		x = tk->pos + 1;
		if (s[x] && s[x] == '=')
			++x;
	}
	else if (s[tk->pos] == '\'') {
		t = token_new(T_SINGLE_QUOTE);
		x = tk->pos + 1;
	}
    else if (s[tk->pos] == ';') {
        /* We're at a comment so we can just ignore the rest of the line */
        tk->pos = strlen(tk->curr_line);
		return token_new(T_COMMENT);
    }
	else if(s[tk->pos] == ')') {
		t = token_new(T_LIST_END);
		x = tk->pos + 1;
	}
	else if (s[tk->pos] == '#') {
		t = token_new(T_CONSTANT);
		x = tk->pos + 1;
		while (s[x] != '\0' && isalpha(s[x]))
			++x;
	}
	else if (is_valid_in_symbol(s[tk->pos])) {
		t = token_new(T_SYM);
		x = tk->pos + 1;
		while (s[x] != '\0' && is_valid_in_symbol(s[x]))
			++x;
	}
	else {
		t = token_new(T_UNKNOWN);
		x = tk->pos + 1;
	}

    int len = x - tk->pos + 1;
	t->val = malloc(len * sizeof(char));
	memcpy(t->val, &s[tk->pos], len - 1);
	t->val[len - 1] = '\0';
	tk->pos = x;

	if (t->type == T_SYM && is_number_token(t)) {
		t->type = T_NUM;
	}

    return t;
}

void tokenizer_stash (tokenizer* tk, token* t) {
	if (tk->stashed)
		t->next = tk->stashed;

	tk->stashed = t;
}

token* next_token(tokenizer* tk) {
	if (tk->stashed) {
		token *t = tk->stashed;
		tk->stashed = tk->stashed->next;
		return t;
	}

	/* If our source is a file, do we need to read another line in? */
	if (tk->file && (!tk->curr_line || tk->curr_line[tk->pos] == '\0')) {
		tokenizer_feed_line(tk, fetch_next_line(tk));
	}

	if (!tk->curr_line)
		return NULL;

	return next_in_line(tk);
}

int start_file(tokenizer *tz, char *filename) {
	tz->file = fopen(filename, "r");

	if (tz->file) {
		return 1;
	}

	return 0;
}
