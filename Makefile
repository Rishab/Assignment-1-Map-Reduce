all: index

index: index.c
	gcc -Wall -Werror -fsanitize=address index.c -o invertedIndex

clean:
	rm -rf invertedIndex
