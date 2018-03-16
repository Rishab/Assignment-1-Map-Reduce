all: invertedIndex

invertedIndex: invertedIndex.c
	gcc -Wall -Werror -fsanitize=address invertedIndex.c -o invertedIndex

clean:
	rm -rf invertedIndex
