CC=cc
CFLAGS= -std=c99 -Wall -ledit
LIBS= -ledit
FILES= notion.c parser.c
OUTPUT= notion

default: notion

#%.o: %.c $(HEADERS)
#	$(CC) $(CFLAGS) -c $< -o $@

notion:
	$(CC) $(CFLAGS) $(LIBS) $(FILES) -o $(OUTPUT)

clean:
		-rm -f *.o
		-rm -f $(OUTPUT)

run: notion
		./notion
