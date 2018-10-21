#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef _WIN32
#include <editline/readline.h>
#endif

#include "environment.h"
#include "evaluator.h"
#include "parser.h"
#include "sexpr.h"
#include "tokenizer.h"

int is_whitespeace(char *s) {
	while (*s) {
		if (!isspace(*s++))
			return 0;
	}

	return 1;
}

#define MAX_LINE_LENGTH 999

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.4.0");
	puts("Press Ctrl-C or (quit) to exit");

	puts("Loading env...");
	scheme_env *env = env_new();
	load_built_ins(env);
	tokenizer *tz = tokenizer_create();
	parser *p = parser_create();
	token *nt;
	int okay;

	char *line;
	while (1) {
#ifdef _WIN32
		line = malloc(sizeof(char) * (MAX_LINE_LENGTH + 1));
		printf("> ");
		fgets(line, MAX_LINE_LENGTH, stdin);
		char *p = strchr(line, '\n');
		if (p) {
			*p = '\0';
		}
#else
		line = readline("> ");

		add_history(line);
#endif
		/* Read in an expression from the user */
		okay = 1;
		if (is_whitespeace(line)) {
			free(line);
			putchar('\n');
			continue;
		}

		tokenizer_feed_line(tz, line);
		nt = next_token(tz);
		while (nt) {
			parser_feed_token(p, nt);
			if (p->complete)
				break;
			token_free(nt);
			nt = next_token(tz);
		}
		token_free(nt);

		if (!p->complete) {
			puts("Incomplete expression. Did you type all your )s?");
			parser_clear(p);
			okay = 0;
		}
		else if (p->head->type == LVAL_ERR) {
			sexpr_pprint(p->head);
			parser_clear(p);
			okay = 0;
		}
		free(line);

		if (!okay)
			continue;

		sexpr *ast = sexpr_copy(p->head);
		parser_clear(p);

		sexpr *result = eval2(env, ast);
		sexpr_free(ast);

		if (result->type == LVAL_ERR && strcmp(result->err, "<quit>") == 0) {
			puts("Notion exiting.");
			sexpr_free(result);
			break;
		}

		sexpr_pprint(result);
		putchar('\n');
		sexpr_free(result);
	}

	tokenizer_free(tz);
	parser_free(p);
	env_free(env);

	return 0;
}
