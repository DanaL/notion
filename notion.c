#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <editline/readline.h>
#endif
#include "parser.h"
#include "evaluator.h"

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.0.0.2");
	puts("Press Ctrl-C to exit");

	while (1) {
		char *line;
		
#ifdef _WIN32
		line = malloc(sizeof(char) * 1000);
		fgets(line, 200, stdin);
		char *p = strchr(line, '\n');
		if (p) {
			*p = '\0';
		}
#else
		line = readline(">");

		add_history(line);
#endif
		lval *ast = parse(line);
		lval *result = lval_eval(ast);

		lval_pprint(result, 0);
		putchar('\n');
		free(result);

		free(ast);
		free(line);
	}

	return 0;
}
