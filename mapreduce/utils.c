/*
 * utils.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>

#include "structures.h"
#include "utils.h"


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


/* function to output the contents of the input list to a document named in the input;
 * if numbers == 1, the list's "words" will simply be written to the file
 * otherwise, the "words" will be written, followed by a tab chartacter and their count
 * NOTE: word lists should ouput each entry on its own line as "WORD\tCOUNT " and number lits as "NUMBER"
 * additionally, a newline does NOT appear at the very bottom of the file (after final entry)
 */
int output_list(LinkedList* list, char* filename, int numbers) {

  // creates the file pointer by opening the file (if it exits) or creating it (if it does not yet exist)
  // also clears existing file data upon open
  remove(filename); //TODO this should be unnecessary, because of the below O_TRUNC flag, but the file is not cleared as expected upon open below, so this step simply deletes the file first if it exists
  int output_file = open(filename, O_WRONLY|O_TRUNC|O_CREAT, 777);

  Node* tmp = list->head;
  int i;
  for (i = 0; i < list->size; i++) {
    
    // writes the current word in the list to the file
    write(output_file, tmp->word, strlen(tmp->word));

    // finds and writes the count of the given string, if not dealing only with sorting integers
    if (numbers != 1) {
      int count_str_len = snprintf(NULL, 0, "%d", tmp->count) + 1;
      char* count_str = (char*) malloc(count_str_len);
      snprintf(count_str, count_str_len, "%d", tmp->count);
      write(output_file, "\t", 1);
      write(output_file, count_str, strlen(count_str));
      free(count_str);
    }

    // prints a \n to prepare for the next output, if this isn't the last one
    if (i < list->size - 1) {
      write(output_file, "\n", 1);
    }

    tmp = tmp->next;
  }

  // closes the output_file pointer
  close(output_file);
  return 0;
}

/* Sorting method utilized by qsort in the build_reduce function (utilizes strcmp) */
int comp(const void *x, const void *y) {
    Node* a = *(Node**)x;
    Node* b = *(Node**)y;    

    return(strcmp(a->word, b->word));
}

/* Sorts the given map table into a reduce table of the required size/shape,
 * given the number of maps performed and the number of reduces to perform
 */
LinkedList** build_reduce(LinkedList** map_table, int num_maps, int num_reduces) {
  
  // discovers the number of strings/numbers stored in the map table
  int num_items = 0;
  int i;
  for (i = 0; i < num_maps; i++) {
    num_items += map_table[i]->size;
  }
  // printf("The map table contains %d items\n", num_items);

  // creates one Node* array for the output of the map_table to live in
  Node** map_output = (Node**) malloc(num_items * sizeof(Node*));
  int j;
  int k = 0;
  for (i = 0; i < num_maps; i++) {
    Node* ptr = map_table[i]->head;
    for (j = 0; j < map_table[i]->size; j++) {
      Node* tmp = (Node*) malloc(sizeof(Node));
      tmp->word = ptr->word;
      tmp->count = ptr->count;
      map_output[k] = tmp;
      ptr = ptr->next;
      // printf("map_output[%d]:: word: %s, count: %d\n", k, map_output[k]->word, map_output[k]->count);
      k++;
    }
  }

  // sorts the one big array holding all the map table's output
  qsort(map_output, num_items, sizeof(Node*), comp);

  // calculates how many items to place into each of the reduce_table's buckets, and the amount of runoff
  int runoff = num_items % num_reduces;
  int size_per_bucket = num_items / num_reduces;

  // allocates the space for the reduce_table, and for each list within
  LinkedList** reduce_table = (LinkedList**) malloc(sizeof(LinkedList*) * num_reduces);

  for (i = 0; i < num_reduces; i++) {
    reduce_table[i] = (LinkedList*) malloc(sizeof(LinkedList));
    reduce_table[i]->size = size_per_bucket;

    if (runoff > 0) {
      reduce_table[i]->size += 1;
      runoff--;
    }
  }

  // fills the reduce_table with entries from the map table in sorted order
  k = 0;
  for (i = 0; i < num_reduces; i++) {
    for (j = 0; j < reduce_table[i]->size; j++) {
      // places the correct item in the reduce_table
      if (reduce_table[i]->head == NULL) {
        Node* tmp = (Node*) malloc(sizeof(Node));
        tmp->word = map_output[k]->word;
        tmp->count = map_output[k]->count;
        reduce_table[i]->head = tmp;
      } else {
        insert_node(reduce_table[i], map_output[k]->word, map_output[k]->count, 0);
      }

      k++;
    }
  }

  // frees the map_output array and its contents
  for (i = 0; i < num_items; i++) {
    free(map_output[i]);
  }
  free(map_output);
  
  return reduce_table;
}
