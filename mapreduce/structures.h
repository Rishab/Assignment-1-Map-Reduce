/*
 * structures.h
 * 
 * Wrapping all of our linked list operations into one file so we can use them
 * anywhere.
 */


#ifndef STRUCTURES_H
#define STRUCTURES_H


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


/* ReduceArgs struct for passing things to threads 
 * Contains the LinkedList that we want to reduce on as well as an integer
 * code for the app (0 for wordcount, 1 for sorting).
 */
typedef struct _ReduceArgs {
    LinkedList *list;
    int app;
} ReduceArgs;

void str_exit(int status, char* message);

Node* traverse(LinkedList* list, int action);

int print_table(LinkedList** table, int size);

int free_table(LinkedList** table, int size);

Node* create_node(char* str, int count);

LinkedList* create_list(char* str, int count);

LinkedList *create_empty_list();

int insert_node (LinkedList* list, char* str, int count, int affect_size);

LinkedList *concat_lists(LinkedList *list_a, LinkedList *list_b);

#endif
