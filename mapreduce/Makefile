CC := gcc
CFLAGS := -Wall -g

all: mapred

parsers.o: parsers.c parsers.h
	$(CC) parsers.c -c -o parsers.o

mapred: mapred.c parsers.c structures.c
	$(CC) $(CFLAGS) mapred.c parsers.c structures.c utils.c -lpthread -lrt -o mapred

clean:
	rm -f parsers.o mapred
