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
#define DEFAULT_TABLE_SIZE 1019

int main(int argc, char **argv) {
	puts("Notion (Dana's toy Scheme) 0.4.0");
	puts("Press Ctrl-C or (quit) to exit");

	puts("Loading env...");

	vm_heap *vm = vm_new();
	scope *global =  scope_new(DEFAULT_TABLE_SIZE);

	load_built_ins(global);
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
		char *cp = strchr(line, '\n');
		if (cp) {
			*cp = '\0';
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
			parser_feed_token(vm, p, nt);
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

		sexpr *ast = p->head;
		parser_clear(p);

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

	return 0;
}
