#ifndef tokenizer_h
#define tokenizer_h

enum token_type { T_NUM, T_SYM, T_STR, T_NULL, T_LIST_START, T_LIST_END,
	T_UNKNOWN, T_CONSTANT, T_SINGLE_QUOTE, T_ERR, T_COMMENT };

typedef struct token {
	enum token_type type;
	char *val;
} token;

token* token_new(enum token_type);
void token_free(token *);
void dump_tokens(char *);

typedef struct tokenizer {
    char *curr_line;
    int pos;
	FILE *file;
} tokenizer;

tokenizer* tokenizer_new(void);
void tokenizer_free(tokenizer*);
void tokenizer_feed_line(tokenizer*, char*);
token* next_token(tokenizer*);
void print_token(token*);
int start_file(tokenizer*, char*);

#endif
