all: invertedIndex
	gcc -Wall -fsanitize=address -o invertedIndex index.c

invertedIndex: index.c

clean:
	rm -rf invertedIndex
