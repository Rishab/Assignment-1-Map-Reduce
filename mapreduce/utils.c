/*
 * utils.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

int sort_comp(const void *x, const void *y) {
    Node *a = *(Node**)x;
    Node *b = *(Node **)y;
    int a_int = atoi(a->word);
    int b_int = atoi(b->word);
    return a_int > b_int;
}

/* Sorts the given counted mapped_list into a reduce_table of the required size/shape,
 * given the number of maps performed and the number of reduces to perform
 */
LinkedList** build_reduce(LinkedList* mapped_list, int num_reduces, int app) {

  // discovers the number of strings/numbers stored in the mapped_list
  int num_items = mapped_list->size;

  // printf("The map table contains %d items\n", num_items);

  // creates one Node* array for the output of the mapped_list to live in
  Node** map_output = (Node**) malloc(num_items * sizeof(Node*));
  int i;
  int j = 0;
  Node* ptr = mapped_list->head;
  for (i = 0; i < num_items; i++) {
    Node* tmp = (Node*) malloc(sizeof(Node));
    tmp->word = ptr->word;
    tmp->count = ptr->count;
    map_output[j] = tmp;
    ptr = ptr->next;
    // printf("map_output[%d]:: word: %s, count: %d\n", k, map_output[k]->word, map_output[k]->count);
    j++;
  }

  // sorts the one big array holding all the map table's output
  if (app == 0) {
    qsort(map_output, num_items, sizeof(Node*), comp);
  } else {
    qsort(map_output, num_items, sizeof(Node*), sort_comp);
  }




  // calculates how many items to place into each of the reduce_table's buckets, and the amount of runoff
  int runoff = num_items % num_reduces;
  int size_per_bucket = num_items / num_reduces;

  // allocates the space for the reduce_table, and for each list within
  LinkedList** reduce_table = (LinkedList**) malloc(sizeof(LinkedList*) * num_reduces);
  /*
  for (i = 0; i < num_reduces; i++) {
    reduce_table[i] = (LinkedList*) malloc(sizeof(LinkedList));
    reduce_table[i]->size = size_per_bucket;

    if (runoff > 0) {
      reduce_table[i]->size += 1;
      runoff--;
    }
  }
  */
  // fills the reduce_table with entries from the map table in sorted order
  int k = 0;
  for (i = 0; i < num_reduces; i++) {
    reduce_table[i] = create_empty_list();


    // Get size of reduce table entry.
    int reduce_size = size_per_bucket;
    if (runoff > 0) {
        reduce_size++;
        runoff--;
    }


    for (j = 0; j < reduce_size; j++) {
      // places the correct item in the reduce_table
      insert_node(reduce_table[i], map_output[k]->word, map_output[k]->count, 1);


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

void print_memory(unsigned char *array, int size) {
    printf("Number of words: %d\n", (int) array[0]);

    int i;
    for (i = 0; i < size; i++) {
        printf("%d  ", array[i]);
    }
    printf("\n");
}

/*
 * Turns a linked list of words and counts into a contiguous array.
 * Organization of the array:
 * First 4 bytes are metadata, tell the # of words in the array.
 * After first 4 bytes (aka 1 int) , we have a bunch of blocks,
 * each corresponding with a word.
 * Each block has:
 *  - 4 bytes (1 int) to tell the size of the block
 *  - 4 bytes (1 int) to tell the count of the word
 *  - variable # of bytes for the null terminated word.
 */
unsigned char *list_to_array(LinkedList *list)
{
    // Traverse list to find the size of the array we need.
    if (!list) { return NULL; }

    int num_words = list->size;
    printf("The number of words in the list according to list to array is: %d\n", num_words);
    int size = sizeof(int) * 2;         // Need at least two ints of metadata.

    Node *ptr = list->head;
    while (ptr) {
        // Need two int's of space for the size and count,
        // and space for the word + 3 for a null terminators.
        size += 2 * sizeof(int) + strlen(ptr->word) + 1;
        ptr = ptr->next;
    }

    printf("Size of list: %d\n", size);

    unsigned char *array = (unsigned char *) calloc(size, size);

    int i = 0;          // Indexes array so we can set different bytes of it.
    int_to_bytes(size, array);
    i += 4;
    int_to_bytes(num_words, array + i);

    i += sizeof(int);

    // Traverse the linked list one more time, putting the 'block' for
    // each word into the array.
    int block_size;
    int word_length;
    ptr = list->head;
    while (ptr) {
        word_length = strlen(ptr->word);
        block_size = 2 * sizeof(int) + word_length + 1;

        array[i] = block_size;
        i += sizeof(int);

        array[i] = ptr->count;
        i += sizeof(int);

        strncpy(array + i, ptr->word, word_length);
        i += word_length + 1;

        ptr = ptr->next;
    }

    print_memory(array, size);

    return array;
}

unsigned char * table_to_array(LinkedList ** reduce_table, int * reduce_size, int num_reduces) {
  // Traverse list to find the size of the array we need.
  if (!reduce_table) { return NULL; }

  int num_words = 0; //Number of words in the list
  int size = sizeof(int) * 2; //the size of the list in bytes
  int i;
  int j;
  for (i = 0; i < num_reduces; i++) {
    Node * ptr = reduce_table[i]->head;
    j = 0;
    while (ptr != NULL && j < *(&reduce_size[i])) {
      // Need two int's of space for the size and count,
      // and space for the word + 3 for a null terminator
      size += 2 * sizeof(int) + strlen(ptr->word) + 1;
      ptr = ptr->next;
      num_words++;
      j++;
    }
  }

  printf("Size of list: %d\n", size);

  unsigned char * array = (char *) calloc(size, size);

  i = 0;          // Indexes array so we can set different bytes of it.
  int_to_bytes(size, array);
  i += 4;
  int_to_bytes(num_words, array + i);

  i += sizeof(int);

  // Traverse the reduce table one more time, putting the 'block' for
  // each word into the array.
  int block_size;
  int word_length;
  int index = 0;
  //int k = 0;
  for (i; i < size; i) {
    Node * ptr = reduce_table[index]->head;
    j = 0;
    while (ptr != NULL && j < *(&reduce_size[index])) {

        word_length = strlen(ptr->word);

        block_size = 2 * sizeof(int) + word_length + 1;

        array[i] = block_size;
        i += sizeof(int);

        array[i] = ptr->count;
        i += sizeof(int);

        strncpy(array + i, ptr->word, word_length);

        i += word_length + 1;

        ptr = ptr->next;
      }
      index++;
    }

  print_memory(array, size);

  return array;
}

/*
 * Function that takes 4 bytes of memory and turns them into an
 * int. Assumed to be Little Endian memory.
 * VERY VERY UNSAFE
 */
int bytes_to_int(unsigned char *c)
{
    return c[0] + (c[1] * 256) + (c[2] * 256 * 256) + (c[3] * 256 * 256 * 256);
}

/*
 * Decomposes the integer x into bytes, and puts in the 4 bytes starting
 * from c.
 * VERY VERY UNSAFE
 */
void int_to_bytes(int x, unsigned char *c)
{
    c[3] = x / (256 * 256 * 256);
    
    x = x % (256 * 256 * 256);
    c[2] = x / (256 * 256);

    x = x % (256 * 256);
    c[1] = x / 256;

    x = x % 256;
    c[0] = x;
}

LinkedList *array_to_list(unsigned char *arr)
{
    int i = 4;
    int num_words = bytes_to_int(arr + i);
    i += 4;
    printf("num_words: %d\n", num_words);

    LinkedList *list = create_empty_list();

    int j;
    int block_size;
    int count;
    for (j = 0; j < num_words; j++) {
        block_size = bytes_to_int(arr + i);
        i += 4;
        count = bytes_to_int(arr + i);
        i += 4;
        char *word = (char *) malloc(block_size - 8);
        
        if (!word) {
            printf("cannot allocate memory when j = %d\n", j);
            exit(1);
        }
        
        strncpy(word, (char *) arr + i, block_size - 8);
        i += block_size - 8;
        //printf("array word: %s, length: %d\n", word, list->size);
        insert_node(list, word, count, 1);
        //traverse(list, 1);
    }

    return list;
}

LinkedList *combine(LinkedList *list) {
    int i;
    int synonyms;

    // creates output list
    LinkedList* reduced_list = create_empty_list();

    // If no linked list, then return NULL.
    if (list == NULL) {
        return NULL;
    }

    // gets pointer to output list head, and checks if it is empty
    Node* ptr_a = list->head;
    if (ptr_a == NULL) {
        return NULL;
    }

    // If there is only one thing in the input, we don't have to combien
    // and can just return the input back.
    if (ptr_a->next == NULL) {
        return list;
    }

    // ptr_b is the "further" of the two list pointers, which will always be one element later than ptr_a
    Node* ptr_b = ptr_a->next;
    // that's why i starts at 1: because it represents the position of ptr_b
    for (i = 1; i < list->size; i++) {
        synonyms = 0;
        // inserts whatever ptr_a is, with its count
        insert_node(reduced_list, ptr_a->word, ptr_a->count, 1);

        // checks for equality of strings, and makes sure the index of the list isn't too far
        while (i < list->size && strcmp(ptr_a->word, ptr_b->word) == 0) {
            // adds count of ptr_b to count of the most-recently-inserted node in the output list
            traverse(reduced_list, 0)->count += ptr_b->count;

            // increments all the pointers and counters
            ptr_a = ptr_b;

            if (ptr_b == NULL) {
                break;
            }
            ptr_b = ptr_b->next;
            synonyms++;
            i++;
        }

        if (synonyms == 0 && i == list->size - 1) {
            // this is hit when the last element of the list was found to be a different word than the second-to-last element
            // it simply adds the word to the output list with its original count (because a word ptr_b is otherwise never added; ptr_b usually only contributes counts)
            insert_node(reduced_list, ptr_b->word, ptr_b->count, 1);
        } else {
            // this is hit most of the time; the pointers are simply updates
            ptr_a = ptr_b;
            if (ptr_b == NULL) {
                break;
            }
            ptr_b = ptr_b->next;
        }
    }

    return reduced_list;
}
