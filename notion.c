#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "parser.h"

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.0.0.0.1");
	puts("Press Ctrl-C to exit");

	while (1) {
		char *line = readline(">");

		add_history(line);

		lval *ast = parse(line);
		lval_pprint(ast, 0);
		free(ast);
		free(line);
	}
	return 0;
}

#include "notion.h"