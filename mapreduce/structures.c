/*
 * structures.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "structures.h"

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

LinkedList *create_empty_list() {
  LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));

  if (!list) {
    return NULL;
  }

  list->head = NULL;
  list->size = 0; 

  return list;
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
