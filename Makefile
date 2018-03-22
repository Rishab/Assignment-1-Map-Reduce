all: index

index: index.c
	gcc -Wall -Werror -fsanitize=address index.c -o index

clean:
	rm -rf index
