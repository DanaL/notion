#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef _WIN32
#include <editline/readline.h>
#endif

#include "parser.h"
#include "environment.h"
#include "evaluator.h"
#include "sexpr.h"

int is_whitespeace(char *s) {
	while (*s) {
		if (!isspace(*s++))
			return 0;
	}

	return 1;
}

#define MAX_LINE_LENGTH 999

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.3.0");
	puts("Press Ctrl-C or (quit) to exit");

	puts("Loading env...");
	scheme_env *env = env_new();
	load_built_ins(env);

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
		line = readline(">");

		add_history(line);
#endif
		int c = 0;
		sexpr *ast = parse(line, &c);

		if (ast->type == LVAL_ERR) {
			sexpr_pprint(ast);
			putchar('\n');
			sexpr_free(ast);
			continue;
		}

		if (is_whitespeace(line))
			putchar('\n');
		else if (ast->type == LVAL_ERR) {
			sexpr_free(ast);
		}
		else
		{
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

		free(line);
	}

	env_free(env);

	return 0;
}
