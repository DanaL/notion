#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <editline/readline.h>
#endif

#include "environment.h"
#include "evaluator.h"
#include "parser.h"
#include "sexpr.h"
#include "tokenizer.h"
#include "util.h"

#define MAX_LINE_LENGTH 999
#define DEFAULT_TABLE_SIZE 1019

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.8.0");
	puts("Press Ctrl-C or (quit) to exit");

	puts("Loading start-up environment...");

	srand(time(NULL));

	vm_heap *vm = vm_new();
	scope *global =  scope_new(DEFAULT_TABLE_SIZE);

	load_built_ins(global);
	tokenizer *tz = tokenizer_new();
	parser *p = parser_new(tz);

	char *line;
	while (1) {
#ifdef _WIN32
		line = malloc(sizeof(char) * (MAX_LINE_LENGTH + 1));
		printf("> ");
		fgets(line, MAX_LINE_LENGTH, stdin);
		char *cp = strchr(line, '\n');
		if (cp) {
			*cp = '\0';
		}
#else
		line = readline("> ");

		add_history(line);
#endif
		/* Read in an expression from the user */
		if (is_whitespace(line)) {
			free(line);
			putchar('\n');
			continue;
		}

		tokenizer_feed_line(tz, line);
		sexpr *ast = get_next_expr(vm, p);
		free(line);

		if (ast->type == LVAL_ERR) {
			sexpr_pprint(ast);
			putchar('\n');
			continue;
		}

		sexpr *result = eval2(vm, global, ast);

		if (result->type == LVAL_ERR && strcmp(result->err, "<quit>") == 0) {
			puts("Notion exiting.");
			break;
		}

		sexpr_pprint(result);
		putchar('\n');
		puts("\nGarbage check:");
		gc_run(vm, global);
	}

	tokenizer_free(tz);
	parser_free(p);
	scope_free(global);
	vm_free(vm);

	return 0;
}
