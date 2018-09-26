CC := gcc
CFLAGS := -Wall -Werror -g

all: mapred

parsers.o: parsers.c parsers.h
	$(CC) parsers.c -c -o parsers.o

mapred: mapred.c parsers.o
	$(CC) $(CFLAGS) mapred.c parsers.c -o mapred

clean:
	rm -f parsers.o mapred
