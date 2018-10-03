/*
 * combine.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* Node struct for the linked lists */
typedef struct _Node {
  char* word;
  int count;
  struct _Node* next;
} Node;

/*
 * Container for the linked list
 * Stores the size of the linked list and its head
 */
typedef struct _LinkedList {
    int size;
    Node *head;
} LinkedList;

/* Helper function to end the program with a message */
void str_exit(int status, char* message) {
  printf("ERROR: %s\nExiting...\n", message);
  exit(status);
}

/* Function to traverse the linked list whose head pointer is the ->head of the input parameter list and return a pointer to the last node
 * NOTE: if action == 1, each node's contents will be printed to standard output; if action == 2, they are all freed
 */
Node* traverse(LinkedList* list, int action) {
  if (list == NULL) {
    printf("The list to traverse is NULL\n");
    return NULL;
  }
  Node* head = list->head;

  if (head == NULL) {
    printf("The head of the list to traverse is NULL\n");
    return NULL;
  }

  if (head->next == NULL) {
    // the head of the list is the last item in the list
    if (action == 1) {
      printf("word: %s, count: %d\n", head->word, head->count);
    } else if (action == 2) {
      free(head);
    }
    return head;
  }

  Node* ptr = head;
  Node* tmp;
  int i;
  for (i = 0; i < list->size; i++) {
    if (action == 1) {
      printf("word: %s, count: %d\n", ptr->word, ptr->count);
    } else if (action == 2) {
      // printf("FREEING word: %s, count: %d\n", ptr->word, ptr->count);
      tmp = ptr;
      free(ptr);
      ptr = tmp;
    }

    if (i == list->size - 1) {
      break;
    }
    ptr = ptr->next;
  }
  return ptr;
}

/* Utility function to print the contents of the entire table, and all its lists, whose pointer is the input parameter 
 * NOTE: the table's first entry must not be NULL, otherwise its size cannot be determined!
 */
int print_table(LinkedList** table, int size) {
  printf("print_table found the array's size to be %d items\n", size); //TODO remove print statement
  
  if (size == 0) {
    printf("WARNING: Could not determine size to print table, or size is zero\n");
    return 1;
  }

  int i;
  for (i = 0; i < size; i++) {
    printf("list %d has %d elements\n", i, table[i]->size);
    traverse(table[i], 1);
    printf("\n");
  }
  return 0;
}

/* Function to free() the entire table, and all its lists, whose pointer is the input parameter 
 */
int free_table(LinkedList** table, int size) {
  // printf("free_table found the array's size to be %d items\n", size); //TODO remove print statement
  
  if (size == 0) {
    printf("WARNING: Could not determine size to free table, or size is zero\n");
    return 1;
  }

  int i;
  for (i = 0; i < size; i++) {
    traverse(table[i], 2);
    free(table[i]);
  }
  free(table);
  return 0;
}

/* Function to create and return a new Node with the given parameters */
Node* create_node(char* str, int count) {
  Node* new_node = (Node*) malloc(sizeof(Node));
  new_node->word = str;
  new_node->count = count;
  new_node->next = NULL;
  return new_node;
}

/* Function to create and return a new LinkedList whose head has the given parameters */
LinkedList* create_list(char* str, int count) {
  LinkedList* new_list = (LinkedList*) malloc(sizeof(LinkedList));
  Node* head = create_node(str, count);
  new_list->head = head;
  new_list->size = 1;

  return new_list;
}

/* Function to insert a new node at the end of the existing linked-list, whose head pointer is the input parameter;
 * if affect_size == 0, the metadata regarding the list's size will not be changed, otherwise it will be incremented upon insert
 */
int insert_node (LinkedList* list, char* str, int count, int affect_size) {
  if (list == NULL) {
    str_exit(0, "Attempted insert into a NULL list!\n");
  }

  Node* head = list->head;
  if (head == NULL) {
    str_exit(0, "Attempted insert into a list whose head is NULL!\n");
  }

  Node* tail = traverse(list, 0);
  if (tail == NULL) {
    str_exit(0, "Strange list behavior; tail node found to be NULL\n");
  }

  Node* temp = create_node(str, count);
  if (temp == NULL) {
    str_exit(0, "Node creation failed!\n");
  }

  tail->next = temp;
  
  if (affect_size != 0) {
    list->size += 1;
  }
  return 0;
}

/* Sorting method utilized by qsort in the build_reduce function (utilizes strcmp) */
/* TODO implement a working comparison method for qsort
  int comp(const void* x, const void* y) {
  Node* x = *(Node**) x;
  Node* y = *(Node**) y;
  return strcmp(x->word, y->word);
}
*/

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

/* Testing method to make sure everything runs correctly (delete when done) */
int main(int argc, char **argv) {

  // designates the name for the output file
  char* outfile = "output";

  // creates/populates the input reduced_list
  LinkedList* reduced_list = create_list("bus", 3);
  insert_node(reduced_list, "car", 2, 1);
  insert_node(reduced_list, "train", 2, 1);
  insert_node(reduced_list, "plane", 2, 1);

  printf("Printing contents of reduced_list...\n");
  traverse(reduced_list, 1);

/* Setup code complete; the above part of this method sets up an expected input from the real program. The below code runs the same way it would in the final program */

  // outputs the reduces_list to the specified file
  output_list(reduced_list, outfile, 0);

  // frees reduced_list
  traverse(reduced_list, 2);

  return 0;
}

