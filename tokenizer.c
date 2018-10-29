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

    return t;
}

void tokenizer_free(tokenizer* t) {
	if (t->file)
		fclose(t->file);

    free(t);
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

	strtol(tk->val, &p, 10);
	if (strcmp(p, "") == 0)
		return 1;
	strtof(tk->val, &p);
	if (strcmp(p, "") == 0)
		return 1;

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

int is_whitespace(char *s) {
	while (*s) {
		if (!(isspace(*s++) || *s == '\n'))
			return 0;
	}

	return 1;
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

token* next_in_line(tokenizer *t) {
	token *tk;
    t->pos = skip_whitespace(t->curr_line, t->pos);
    char *s = t->curr_line;
    int x = t->pos;

	if (s[t->pos] == '\0') {
		free(t->curr_line);
		t->curr_line = NULL;
		t->pos = 0;

		return NULL;
	}

	if (t->curr_line[t->pos] == '(') {
        tk = token_new(T_LIST_START);
        tk->val =  n_strcpy(tk->val, "(");
        t->pos++;

        return tk;
    }

    if (s[t->pos] == '+' || s[t->pos] == '*' || s[t->pos] == '/' || s[t->pos] == '%'
			|| s[t->pos] == '=' || s[t->pos] == '^'
			|| (s[t->pos] == '-' && s[t->pos] == ' ')) {
		tk = token_new(T_SYM);
		x = t->pos + 1;
	}
	else if (s[t->pos] == '\\') {
		tk = token_new(T_ERR);
		tk->val = NULL;
		tk->val = n_strcpy(tk->val, "Invalid token.");
		return tk;
	}
	else if (s[t->pos] == '"') {
		tk = parse_str_token(s, &(t->pos));
		return tk;
	}
	else if (s[t->pos] == '<' || s[t->pos] == '>') {
		tk = token_new(T_SYM);
		x = t->pos + 1;
		if (s[x] && s[x] == '=')
			++x;
	}
	else if (s[t->pos] == '\'') {
		tk = token_new(T_SINGLE_QUOTE);
		x = t->pos + 1;
	}
    else if (s[t->pos] == ';') {
        /* We're at a comment so we can just ignore the rest of the line */
        t->pos = strlen(t->curr_line);
        return token_new(T_COMMENT);
    }
	else if(s[t->pos] == ')') {
		tk = token_new(T_LIST_END);
		x = t->pos + 1;
	}
	else if (s[t->pos] == '#') {
		tk = token_new(T_CONSTANT);
		x = t->pos + 1;
		while (s[x] != '\0' && isalpha(s[x]))
			++x;
	}
	else if (is_valid_in_symbol(s[t->pos])) {
		tk = token_new(T_SYM);
		x = t->pos + 1;
		while (s[x] != '\0' && is_valid_in_symbol(s[x]))
			++x;
	}
	else {
		tk = token_new(T_UNKNOWN);
		x = t->pos + 1;
	}

    int len = x - t->pos + 1;
	tk->val = malloc(len * sizeof(char));
	memcpy(tk->val, &s[t->pos], len - 1);
	tk->val[len - 1] = '\0';
	t->pos = x;

	if (tk->type == T_SYM && is_number_token(tk))
	 	tk->type = T_NUM;

    return tk;
}

token* next_token(tokenizer* t) {
	/* If our source is a file, do we need to read another line in? */
	if (t->file && (!t->curr_line || t->curr_line[t->pos] == '\0')) {
		tokenizer_feed_line(t, fetch_next_line(t));
	}

	if (!t->curr_line)
		return NULL;

	return next_in_line(t);
}

int start_file(tokenizer *tz, char *filename) {
	tz->file = fopen(filename, "r");

	if (tz->file) {
		return 1;
	}

	return 0;
}
