#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.0.0.9");
	puts("Press Ctrl-C to exit");

	puts("Loading env...");
	scheme_env *env = env_new();
	load_built_ins(env);

	int e = 0;
	sexpr *f0 = parse("(define (sq x) (* x x))", &e);
	eval2(env, f0);
	e = 0;
	sexpr *f1 = parse("(define (sum_of_sq x y) (+ (sq x) (sq y)))", &e);
	eval2(env, f1);
	sexpr_free(f0);
	sexpr_free(f1);

	while (1) {
		char *line;

#ifdef _WIN32
		line = malloc(sizeof(char) * 1000);
		printf("> ");
		fgets(line, 200, stdin);
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

		if (is_whitespeace(line))
			putchar('\n');
		else if (ast->type == LVAL_ERR) {
			sexpr_pprint(ast);
			sexpr_free(ast);
		}
		else
		{
			sexpr *result = eval2(env, ast);

			sexpr_pprint(result);
			putchar('\n');
			sexpr_free(result);

			sexpr_free(ast);
		}

		free(line);
	}

	env_free(env);

	return 0;
}
