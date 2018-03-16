all: invertedIndex
	gcc -Wall -fsanitize=address -o invertedIndex invertedIndex.c

invertedIndex: invertedIndex.c

clean:
	rm -rf invertedIndex
