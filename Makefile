CC=cc
CFLAGS= -std=c99 -Wall -ledit
LIBS= -ledit
FILES= parser.c environment.c evaluator.c
OUTPUT= notion

default: notion

objs:
	$(CC) $(CFLAGS) -c $(LIBS) $(FILES) 

#%.o: %.c 
#	$(CC) $(CFLAGS) -c $< -o $@

notion: objs
	$(CC) $(CFLAGS) $(LIBS) $(FILES) notion.c -o $(OUTPUT)

clean: 
		-rm -f *.o
		-rm -f $(OUTPUT)

run: notion
		./notion
