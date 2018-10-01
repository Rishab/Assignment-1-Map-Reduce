
/*
 * mapred.c
 * Testing program.
 */

#include "mapred.h"

/* Map function: Takes all the words at each index and assigns a count to them.
   If a word is duplicated in the list, then it combines their counts together.
   Whether there is a thread or process required, the list will run the algorithm
   using multiple processes or threads
*/
void map(struct tpTable ** hashmap, int num_maps, int size_of_maps) {
  int i = 0;
  int j = 0;

  //Create pointer to list

  for (i = 0; i < num_maps; i++) {
    for (j = 0; j < size_of_maps; j++) {
      //traverse the entire list and compare it with the initial word for that process/thread
        //combine as necessary
    }
  }
}

/* Utility function that creates nodes to be placed inside the hashmap */
struct tpTable * createHashMapNode(char * word) {
  struct tpTable * temp = (struct tpTable *) malloc(sizeof(struct tpTable)); //create a new node
  temp->word = word; //copy the word from the global linked list to the node
  temp->next = NULL; //set pointer next to null to maintain order
  return temp;
}

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap(struct tpTable ** hashmap, WCLinkedList * list, int num_maps, int size_of_maps) {
  int i = 0;
  int j = 0;

  WCParseNode * list_ptr = list->head; //pointer to the head of the global linked list

  /* loop to traverse the entire linked list and insert nodes to respective indicies */
  while (list_ptr->next != NULL) {

    //loop to traverse every index in the hashmap
    for (i = 0; i < num_maps; i++) {
      struct tpTable * hash_ptr = hashmap[i]; //pointer to the current index
      //loop to insert a number of words into the size of the hashmap

      for (j = 0; j < size_of_maps; j++) {

        //for the case that the current index of the hashmap has no head node
        if (hashmap[i] == NULL) {
          struct tpTable * temp = createHashMapNode(list_ptr->word);
          hashmap[i] = temp;
          hash_ptr = temp;
          //printf("Inserted first node to index: %d\n", i);
        }

        //for the case that we are inserting any node after the head or at the end of the list
        else if (hashmap[i] != NULL) {
          struct tpTable * temp = createHashMapNode(list_ptr->word);
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
void printmap(struct tpTable ** hashmap, int num_maps) {
  struct tpTable * print_ptr = NULL; //pointer to keep track of current node to be printed
  int i = 0;

  //loop to traverse the entire hashmap for printing purposes
  for (i = 0; i < num_maps; i++) {
    print_ptr = hashmap[i];
    printf("%d: ", i);

    //loop to traverse all the nodes at the current index of the list
    while (print_ptr != NULL) {
      printf("%s  %d --> ", print_ptr->word, print_ptr->count);
      print_ptr = print_ptr->next;
    }
    printf("\n");
  }
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
    WCLinkedList *list = word_count_parse(input_file_path);
    print_wc_list(list);

    int size_of_maps = list->size/num_maps; //used to determine how many nodes each thread/process will handle
    printf("The number of words at each index will be: %d\n", size_of_maps);

    //global hashmap used to organize what each thread/process will handle
    hashmap = (struct tpTable **) malloc(sizeof(struct tpTable *) * num_maps);
    int i = 0;
    for (i = 0; i < num_maps; i++) {
      hashmap[i] = NULL;
    }

    fillHashMap(hashmap, list, num_maps, size_of_maps);
    printmap(hashmap, num_maps);

    //map function used to find similar words at the current index and combine their counts
    map(hashmap, num_maps, size_of_maps);
    return 0;
}
