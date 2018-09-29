#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>

/*
 * Gets array of m chunks from the file.
 * If the file is s bytes, this tries to separate the 
 * file into s/m sized chunks, but if that puts us in the middle
 * of a word, it skips to the end of the word before separating.
 *
 * The array is malloc'ed, so it and everything in it will need to 
 * be free'd when parsing.
 */
char **split(char *file, int m)
{
    int fd = open(file, O_RDONLY);
    struct stat st;
    fstat(fd, &st);

    int s = st.st_size;         // Get size of file in bytes.

    printf("file size: %d\n", s);
    
    int i;
    char b[2];
    int start = 0;
    int chunk = 1;
    int chunk_size;

    char **split_points = (char **) malloc(sizeof(char *) * m);
    for (i = 0; i < m; i++) {
        split_points[i] = NULL;
    }
    i = 0;
    while (i < s) {
        read(fd, b, 1);
        if (i < chunk * s/m) {
            i++;
        } else {
            if (!isalnum(b[0])) {
                chunk_size = i - start + 1;
                if (chunk_size > 0) {
                    split_points[chunk-1] = (char *) calloc(sizeof(char *) * chunk_size, sizeof(char *) * chunk_size);
                    pread(fd, split_points[chunk-1], chunk_size-1, start);
                    chunk++;
                    i++;
                    start = i;
                }
            } else {
                i++;
            }
        }
    }
    if (chunk < m + 1) {
        chunk_size = s - start + 1;
        split_points[chunk-1] = (char *) calloc(sizeof(char) * chunk_size, sizeof(char)*chunk_size);
        pread(fd, split_points[chunk-1], chunk_size-1, start);
    }

    return split_points;
}

void print_array(char **arr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        printf("%s, ", arr[i]);
        printf("\n----------------\nEND OF LINE\n-----------------\n\n");
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    char **splits = split("file2.txt", 10);
    print_array(splits, 10);


    return 0;
}
