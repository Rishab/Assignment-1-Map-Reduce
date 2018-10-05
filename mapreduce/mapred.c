
/*
 * mapred.c
 * Testing program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mapred.h"
#include "structures.h"
#include "parsers.h"
#include "utils.h"

TpTable * head;

TpTable ** hashmap;

/* Map function: Takes all the words at each index and assigns a count to them.
   If a word is duplicated in the list, then it combines their counts together.
   Whether there is a thread or process required, the list will run the algorithm
   using multiple processes or threads
*/
void map(TpTable ** hashmap, int num_maps, int size_of_maps) {
  int i = 0;
  int j = 0;

  //Create pointer to list

  for (i = 0; i < num_maps; i++) {
    for (j = 0; j < size_of_maps; j++) {
      //traverse the entire list and compare it with the initial word for that process/thread
      //combine as necessary
      ; 
    }
  }
}

/* Utility function that creates nodes to be placed inside the hashmap */
TpTable * createHashMapNode(char * word) {
  TpTable * temp = (TpTable *) malloc(sizeof(TpTable)); //create a new node
  temp->word = word; //copy the word from the global linked list to the node
  temp->next = NULL; //set pointer next to null to maintain order
  return temp;
}

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap(LinkedList * list, int num_maps, int size_of_maps) {
  int i = 0;
  int j = 0;

  Node * list_ptr = list->head; //pointer to the head of the global linked list

  /* loop to traverse the entire linked list and insert nodes to respective indicies */
  while (list_ptr->next != NULL) {

    //loop to traverse every index in the hashmap
    for (i = 0; i < num_maps; i++) {
      TpTable * hash_ptr = hashmap[i]; //pointer to the current index
      //loop to insert a number of words into the size of the hashmap

      for (j = 0; j < size_of_maps; j++) {

        //for the case that the current index of the hashmap has no head node
        if (hashmap[i] == NULL) {
          TpTable * temp = createHashMapNode(list_ptr->word);
          hashmap[i] = temp;
          hash_ptr = temp;
          //printf("Inserted first node to index: %d\n", i);
        }

        //for the case that we are inserting any node after the head or at the end of the list
        else if (hashmap[i] != NULL) {
          TpTable * temp = createHashMapNode(list_ptr->word);
          hash_ptr->next = temp;
          hash_ptr = temp;
          //printf("Inserted the %d node to the current index: %d\n", j, i);
        }

        list_ptr = list_ptr->next;
        //printmap(hashmap, num_maps);
      }
    }
  }
  printf("Hashmap filling complete!\n");
}

/* Utility function to print out the global hashmap for debugging purposes */
void printmap(TpTable ** map, int num_maps) {
  TpTable * print_ptr = NULL; //pointer to keep track of current node to be printed
  int i = 0;

  //loop to traverse the entire hashmap for printing purposes
  for (i = 0; i < num_maps; i++) {
    print_ptr = map[i];
    printf("%d: ", i);

    //loop to traverse all the nodes at the current index of the list
    while (print_ptr != NULL) {
      printf("%s  %d --> ", print_ptr->word, print_ptr->count);
      print_ptr = print_ptr->next;
    }
    printf("\n");
  }
}

/* reducer function (used by reduce): given list gets assigned to a process or thread to perform the reduce operation,
 * that is, for all the words the list's counts get combined
 * Whether there is a thread or process required, the list will run the algorithm
 * using multiple processes or threads
 */
void* reducer(void* reduce_args) {

  LinkedList* list = ((ReduceArgs*)reduce_args)->list;
  int process = ((ReduceArgs*)reduce_args)->process;

  int i;
  int synonyms;

  if (process == 1) { //TODO make work if process
    list_to_array(list);
  }

  // creates output list
  LinkedList* reduced_list = (LinkedList*) malloc(sizeof(LinkedList));

  // gets pointer to output list head, and checks if it is empty
  Node* ptr_a = list->head;
  if (ptr_a == NULL) {
    printf("ERROR: Attempted to reduce on a list whose head is NULL! Exiting...\n");
    exit(1);
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
      ptr_b = ptr_b->next;
    }
  }

  if (process == 1) { //TODO make work if process
    list_to_array(list);
  }

  pthread_exit((void*) reduced_list);
  return (void*) reduced_list;
}

/* reduce function: wrapper for the reducer() function;
 * reduce spawns threads or processes of reducer()s and returns a LinkedList of the combined input
 */
LinkedList* reduce(LinkedList** reduce_table, int num_reduces, int process) {
  int i;
  LinkedList* reduce_return = (LinkedList*) malloc(sizeof(LinkedList));

  if (process == 1) {
    // run reducer() in a process
    pid_t* pids = (pid_t*) malloc(sizeof(pid_t) * num_reduces);
    
    for (i = 0; i < num_reduces; i++) {
      printf("process!\n"); //TODO
    }
  } else {
    // run reducer() in a thread
    pthread_t* thread_ids = (phtread_t*) malloc(sizeof(pthread_t) * num_reduces);
    for (i = 0; i < num_reduces; i++) {
      ReduceArgs* reduce_args = (ReduceArgs*) malloc(sizeof(ReduceArgs));
      reduce_args->list = reduce_table[i];
      reduce_args->process = process;
      pthread_create(&thread_ids[i], NULL, reducer, (void*) reduce_args);
    }
    for (i = 0; i < num_reduces; i++) {
      void* return_val;
      pthread_join(&thread_ids[i], &return_val); //TODO make sure pthread_join blocks until the thread it is looking at has returned ("exited")
      LinkedList* return_segment = ((LinkedList*) return_val);
      reduce_return = concat_Lists(reduce_return, return_segment);
    }
  }

  return reduce_return;
}

int main(int argc, char **argv) {
    //store command line arguments in variables
    char * program_type = argv[1];
    char * parallel_type = argv[2];
    int num_maps = atoi(argv[3]);
    int num_reduces = atoi(argv[4]);
    char * input_file_path = argv[5];
    char * output_file_path = argv[6];

    //debugging statement for input arguments
    printf("%s %s %d %d %s %s\n", program_type, parallel_type, num_maps, num_reduces, input_file_path, output_file_path);

    //creating the list with all the tokens from the input file
    LinkedList *list = word_count_parse(input_file_path);

    print_table(&list, 1);

    char *array = list_to_array(list);

    LinkedList *list2 = array_to_list(array);

    traverse(list2, 1);

    

    /*
    int size_of_maps = list->size/num_maps; //used to determine how many nodes each thread/process will handle
    printf("The number of words at each index will be: %d\n", size_of_maps);

    //global hashmap used to organize what each thread/process will handle
    hashmap = (TpTable **) malloc(sizeof(struct TpTable *) * num_maps);
    int i = 0;
    for (i = 0; i < num_maps; i++) {
      hashmap[i] = NULL;
    }

    fillHashMap(list, num_maps, size_of_maps);
    printmap(hashmap, num_maps);

    //map function used to find similar words at the current index and combine their counts
    map(hashmap, num_maps, size_of_maps);
    */
    return 0;
}
