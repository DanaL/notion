#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <editline/readline.h>
#endif
#include "parser.h"
#include "evaluator.h"

int is_whitespeace(char *s) {
	while (*s) {
		if (!isspace(*s++))
			return 0;
	}

	return 1;
}

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.0.0.3");
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
		int c = 0;
		sexpr *ast = parse(line, &c);

		if (is_whitespeace(line))
			putchar('\n');
		else 
		{
			sexpr *result = eval(ast);

			sexpr_pprint(result, 0);
			putchar('\n');
			free(result);

			free(ast);
		}

		free(line);
	}

	return 0;
}
