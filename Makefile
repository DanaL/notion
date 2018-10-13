CC=cc
CFLAGS= -std=c99 -Wall -g3
LIBS= -ledit
FILES= parser.c environment.c evaluator.c sexpr.c util.c
OUTPUT= notion

default: notion

objs:
	$(CC) $(CFLAGS) -c $(FILES)

notion: objs
	$(CC) $(CFLAGS) $(LIBS) $(FILES) notion.c -o $(OUTPUT)

clean:
		-rm -f *.o
		-rm -f $(OUTPUT)

run: notion
		./notion
