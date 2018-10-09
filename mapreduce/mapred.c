
/*
 * mapred.c
 * Testing program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "mapred.h"
#include "structures.h"
#include "parsers.h"
#include "utils.h"

TpTable * head;

TpTable ** hashmap;

/*
 * Name that we have to give to shared memory on creating it.
 * We can unlink the shared memory using this name later.
 */
const char *mem_name = "/shared_mem";

/*
 * Our global linked list that we construct in map.
 * If using threads, this is shared amongst the threads.
 */
LinkedList *global_map_list = NULL;


/*
 * Global linked list that holds the results of reduce.
 * Stores intermediate data when being shared across threads.
 */
LinkedList *global_reduce_list = NULL;


/*
 * Mutex that we'll lock before accessing the global_map_list
 * or global_reduce_list.
 */
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;


/*
 * Wrapper function for both the map_processes() and map_threads()
 * functions.
 * First, parse through the input file. Then, build a TpTable array
 * based on the number of maps we need to do. Then, send those off
 * to either processes (if processes == 1) or threads (if
 * processes == 0).
 * Return the result of the mapping operation.
 */
LinkedList *map(char *filename, int processes, int num_maps) {
    //creating the list with all the tokens from the input file
    LinkedList *list = word_count_parse(filename);

    print_table(&list, 1);


    int ll_size = list->size;

    printf("There are %d words in the linked list\n", list->size);

    int * map_size = (int *) malloc(sizeof(int) * list->size);
    map_size = determineMapSize(list->size, num_maps);


    int i;

    for (i = 0; i < num_maps; i++) {
        printf("Map #%d: %d\n", i, map_size[i]);
    }

    printf("About to fill in the hash map\n");
    //global hashmap used to organize what each thread/process will handle
    hashmap = (TpTable **) malloc(sizeof(struct TpTable *) * num_maps);
    for (i = 0; i < num_maps; i++) {
      hashmap[i] = NULL;
    }

    fillHashMap(list, num_maps, map_size);
    printf("Successfully filled in hashmap\n");
    printmap(hashmap, num_maps);

    if (processes) {
        global_map_list = map_processes(hashmap, list, num_maps, ll_size, map_size);
    } else {
        global_map_list = map_threads(hashmap, num_maps);
    }

    return global_map_list;
}

void *map_thread_handler(void *args) {
    printf("in thread\n");

    TpTable *table = (TpTable *) args;
    LinkedList *list = tpTable_to_list(table);

    printf("linked list madei\n");

    // Go through list and set all counts to 1.
    Node *ptr = list->head;
    while (ptr) {
        ptr->count = 1;
        ptr = ptr->next;
    }

    pthread_mutex_lock(&list_mutex);
    printf("locked\n");
    global_map_list = concat_lists(global_map_list, list);
    //traverse(global_map_list, 1);
    pthread_mutex_unlock(&list_mutex);

    pthread_exit(NULL);
    return NULL;
}

/*
 * Mapper function with threads.
 * Just sets counts to 1 and puts the maps back together.
 */
LinkedList *map_threads(TpTable **hashmap, int num_maps) {
    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_maps);
    int i;
    for (i = 0; i < num_maps; i++) {
        printf("starting thread #%d\n", i);
        pthread_create(threads+i, NULL, &map_thread_handler, (void *)hashmap[i]);
    }

    for (i = 0; i < num_maps; i++) {
        pthread_join(threads[i], NULL);
    }

    return global_map_list;
}

LinkedList *tpTable_to_list(TpTable *table) {
    LinkedList *list = create_empty_list();
    TpTable *ptr = table;

    while (ptr) {
        insert_node(list, ptr->word, ptr->count, 1);
        ptr = ptr->next;
    }

    return list;
}

/* Map function: Takes all the words at each index and assigns a count to them.
   If a word is duplicated in the list, then it combines their counts together.
   Whether there is a thread or process required, the list will run the algorithm
   using multiple processes or threads
*/
LinkedList * map_processes(TpTable ** hashmap, LinkedList * list, int num_maps, int array_size, int * map_size) {

  /*Convert Linked List to an array so that we can create a shared memory region that utilizes
  our array for the processes */
  size_t array_length = array_size;
  printf("The length of the input array is: %d\n", array_length);

  char * data_array = (char *) calloc(sizeof(char) * array_length, sizeof(char) * array_length);

  int sharedmem_fd = shm_open(mem_name, O_RDWR | O_CREAT, 0666);

  if (sharedmem_fd < 0) {
    printf("File descriptor failed to open.\n");
  }
  char * temp = list_to_array(list);
  int array_length2 = bytes_to_int(temp);

  ftruncate(sharedmem_fd, array_length2);

  char * ptr = (char *) mmap(&data_array, array_length2, PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem_fd, 0);


  memcpy(ptr, temp, array_length2);
  print_memory(ptr, array_length2);
  printf("Our shared memory region is attached at the following address: %p\n", ptr);

  printf("About to figure out start and end indexes for each process\n");
  int * start_end = startEnd(hashmap, NULL, 0, ptr, array_length2, map_size, num_maps);

  pid_t process_ids[num_maps];

  int i;
  for (i = 0; i < num_maps; i++) {
    process_ids[i] = fork();
    int start_index = start_end[2*i];
    int end_index = start_end[2*i+1];
    if (process_ids[i] < 0) {
      printf("Fork failed");
      printf("Error: %s\n", strerror(errno));
      abort();
    }
    else if (process_ids[i] == 0) {
      printf("\nFork spawned a process sucessfully in map!\n");
      printf("Child Process ID: %d and Parent Process ID: %d\n", getpid(), getppid());
      process_map(start_index, end_index, ptr);
      exit(0);
    }
  }

  int n = num_maps;
  for (i = 0; i < num_maps; i++) {
    wait(NULL);
    printf("Ending Child Process ID: %d and Parent Process ID: %d\n", getpid(), getppid());
  }

  LinkedList *mapped_list = array_to_list(ptr);

  print_table(&mapped_list, 1);
  shm_unlink(data_array);
  free(data_array);
  free(temp);

  return mapped_list;

}

void process_map(int start_index, int end_index, char * shared_memory) {
  int i = start_index;
  printf("Start Index: %d End Index: %d\n", start_index, end_index);
  while (i < end_index) {
    int current_block_size = bytes_to_int(shared_memory + i);
    printf("The current block_size is: %d\n", current_block_size);
    shared_memory[i+4] = 1;
    i += current_block_size;
  }
}

int * determineMapSize(int num_words, int num_maps) {
	int * temp = (int *) malloc(sizeof(int) * num_maps);
	int i;
	if (num_words < num_maps) {
    int temp_words = num_words;
		for (i = 0; i < num_maps && temp_words != 0; i++) {
      temp[i] = 1;
      temp_words--;
    }
    for (i = num_words; i < num_maps; i++) {
      temp[i] = 0;
    }
	}
	else if (num_maps % num_words == 0) {
		int words_per_map = num_maps/num_words;
		for (i = 0; i < num_maps; i++) {
			temp[i] = words_per_map;
		}
	}

	else {
    printf("The number of maps are: %d and words are: %d\n", num_maps, num_words);
    int remainder = num_words % num_maps;
		int words_per_map = (num_words - remainder)/num_maps;
    printf("The words per map are: %d\n", words_per_map);
		for (i = 0; i < num_maps; i++) {
			temp[i] = words_per_map;
		}
    printf("The remainder of words left is: %d\n", remainder);
		for (i = 0; i < num_maps && remainder > 0; i++) {
			temp[i]++;
			remainder--;
		}
	}

	return temp;
}

int * startEnd (TpTable ** map, LinkedList ** reduce_table, int map_or_reduce, char * sharedMemory, int array_length, int * word_sizes, int num_maps_reduces) {
  int * start_end = (int *) malloc(sizeof(int) * 2 * num_maps_reduces);
  if (map_or_reduce == 0) {
    printf("Inside StartEnd function\n");
    int i;
    int j = 8; //keeps track of the current index in the sharedMemory array
    int tracker = 0; //keeps track of which process/thread maps to which indexes in the returned array

    int shm_length = bytes_to_int(sharedMemory);

    printf("SHM length: %d\n", shm_length);
    //traverse the entire map and sharedMemory to determine which processes map to which indexes in the array
    for (i = 0; i < num_maps_reduces; i++) {
      int current_num_words = word_sizes[i];
      printf("The current number of words to discover the chunk for are: %d\n", current_num_words);
      int start = j;
      int end = j;
      //printf("The value of start is %d and end is %d\n", start, end);
      while (j < array_length && current_num_words != 0) {
        int current_block_size = bytes_to_int(sharedMemory + j);
        //printf("The size of the current block is: %d\n", current_block_size);
        j += current_block_size;
        current_num_words--;
      }
      end = j;
      start_end[tracker] = start;
      tracker++;
      start_end[tracker] = end;
      tracker++;
    }

    printf("\n");
    for (i = 0; i < 2 * num_maps_reduces; i++) {
      printf("%d\t", start_end[i]);
    }
    printf("\n");
  } else {
    printf("Inside StartEnd function");
    int i;
    int j = 8; //keeps track of the current index in the sharedMemory array
    int tracker = 0; //keeps track of which process/thread maps to which indexes in the returned array

    int shm_length = bytes_to_int(sharedMemory);

    printf("SHM length: %d\n", shm_length);
    //traverse the entire map and sharedMemory to determine which processes map to which indexes in the array
    for (i = 0; i < num_maps_reduces; i++) {
      int current_num_words = word_sizes[i];
      printf("The current number of words to discover the chunk for are: %d\n", current_num_words);
      int start = j;
      int end = j;
      //printf("The value of start is %d and end is %d\n", start, end);
      while (j < array_length && current_num_words != 0) {
        int current_block_size = bytes_to_int(sharedMemory + j);
        //printf("The size of the current block is: %d\n", current_block_size);
        j += current_block_size;
        current_num_words--;
      }
      end = j;
      start_end[tracker] = start;
      tracker++;
      start_end[tracker] = end;
      tracker++;
    }

    printf("\n");
    for (i = 0; i < 2 * num_maps_reduces; i++) {
      printf("%d\t", start_end[i]);
    }
    printf("\n");

  }

  return start_end;
}

/* Utility function that creates nodes to be placed inside the hashmap */
TpTable * createHashMapNode(char * word, int count) {
  TpTable * temp = (TpTable *) malloc(sizeof(TpTable)); //create a new node
  temp->word = word; //copy the word from the global linked list to the node
  temp->count = count;
  temp->next = NULL; //set pointer next to null to maintain order
  return temp;
}

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap(LinkedList * list, int num_maps, int * map_size) {
  int i = 0;
  int j = 0;

  Node * list_ptr = list->head; //pointer to the head of the global linked list

  /* loop to traverse the entire linked list and insert nodes to respective indicies */
  while (list_ptr != NULL) {
    //loop to traverse every index in the hashmap
    for (i = 0; i < num_maps; i++) {
      TpTable * hash_ptr = hashmap[i]; //pointer to the current index
      //loop to insert a number of words into the size of the hashmap

      for (j = 0; j < map_size[i]; j++) {

        //for the case that the current index of the hashmap has no head node
        if (hashmap[i] == NULL) {
          TpTable * temp = createHashMapNode(list_ptr->word, list_ptr->count);
          hashmap[i] = temp;
          hash_ptr = temp;
          //printf("Inserted first node to index: %d with word: %s\n", i, temp->word);
        }

        //for the case that we are inserting any node after the head or at the end of the list
        else if (hashmap[i] != NULL) {
          TpTable * temp = createHashMapNode(list_ptr->word, list_ptr->count);
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

/* reducer function (used by reduce): given list gets assigned to a thread to perform the reduce operation,
 * that is, for all the words the list's counts get combined.
 * Pass in a ReduceArgs struct, which contains the LinkedList we need to
 * combine on and the app (0 for wordcount, 1 for sort).
 * Note: if app == 1, since we don't have to combine duplicates, this
 * essentially does nothing.
 */
void* reduce_thread_handler(void* reduce_args) {
  ReduceArgs *args = (ReduceArgs *) reduce_args;
  LinkedList* list = args->list;
  int app = args->app;

  if (app == 1) {
    pthread_exit((void *) list);
    return (void *) list;
  }

  int i;

//  printf("Printing input list...\n");
//  traverse(list, 1);

  // creates output list
  LinkedList* reduced_list = create_empty_list();

  // gets pointer to output list head, and checks if it is empty
  Node* ptr_a = list->head;
  if (ptr_a == NULL) {
    pthread_exit(NULL);
    return NULL;
  }

  if (ptr_a->next == NULL) {
    pthread_exit((void *) list);
    return (void *) list;
  }

//  printf("%s\n", list->head->word);

  // ptr_b is the "further" of the two list pointers, which will always be one element later than ptr_a
  Node* ptr_b = ptr_a->next;
  // that's why i starts at 1: because it represents the position of ptr_b
  for (i = 1; i < list->size; i++) {
    // inserts whatever ptr_a is, with its count
    insert_node(reduced_list, ptr_a->word, ptr_a->count, 1);

    // checks for equality of strings, and makes sure the index of the list isn't too far
    while ((i < list->size) && (strcmp(ptr_a->word, ptr_b->word) == 0)) {
      // adds count of ptr_b to count of the most-recently-inserted node in the output list
      traverse(reduced_list, 0)->count += ptr_b->count;

      // increments all the pointers and counters

      if (ptr_b->next != NULL) {
      i++;
      ptr_a = ptr_b;
      ptr_b = ptr_b->next;
      } else {
        break;
      }
    }

    if ((strcmp(ptr_a->word, ptr_b->word) != 0) && i == list->size - 1) {
      // this is hit when the last element of the list was found to be a different word than the second-to-last element
      // it simply adds the word to the output list with its original count (because a word ptr_b is otherwise never added; ptr_b usually only contributes counts)
      insert_node(reduced_list, ptr_b->word, ptr_b->count, 1);
    } else {
      // this is hit most of the time; the pointers are simply updated
      ptr_a = ptr_b;
//  /*
      if (ptr_b == NULL) {
        break;
      }
//  */
      ptr_b = ptr_b->next;
    }
  }

  printf("Reduced list:\n");
  //traverse(reduced_list, 1);
  printf("\n");

  pthread_exit((void *) reduced_list);
  return (void *) reduced_list;
}

int * determineReduceSize(LinkedList ** reduce_table, int num_reduces) {
  int * temp = (int *) malloc(sizeof(int) * num_reduces);
  int i;
  for (i = 0; i < num_reduces; i++) {
    temp[i] = reduce_table[i]->size;
  }
  return temp;
}

LinkedList * reduce_processes(LinkedList ** reduce_table, int * reduce_size, int num_reduces) {
  printf("Reducing the processes!\n");

  /*Convert Linked List to an array so that we can create a shared memory region that utilizes
  our array for the processes */
  char * temp = table_to_array(reduce_table, reduce_size, num_reduces);

  /*determine the length of the array segment */
  int array_length = bytes_to_int(temp);
  printf("The length of the array is: %d\n", array_length);

  /* create the data array used for shared memory */
  char * data_array = (char *) calloc(sizeof(char) * array_length, sizeof(char) * array_length);

  /* get the file descriptor of the shared memory */
  int sharedmem_fd = shm_open(mem_name, O_RDWR | O_CREAT, 0666);

  if (sharedmem_fd < 0) {
    printf("Failed to return a file descriptor\n");
  }

  ftruncate(sharedmem_fd, array_length);

  char * ptr = (char *) mmap(&data_array, array_length, PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem_fd, 0);

  memcpy(ptr, temp, array_length);
  print_memory(ptr, array_length);

  int * start_end = startEnd(NULL, reduce_table, 1, ptr, array_length, reduce_size, num_reduces);

  int i;
  pid_t process_ids[num_reduces];

  for (i = 0; i < num_reduces; i++) {
    process_ids[i] = fork();
    int start_index = start_end[2*i];
    int end_index = start_end[2*i+1];
    if (process_ids[i] < 0) {
      printf("Fork failed in reduce\n");
      printf("Error: %s\n", strerror(errno));
      abort();
    } else if (process_ids[i] == 0) {
      printf("\nFork spawned a process successfully in reduce!\n");
      printf("Child Process ID: %d and Parent Process ID: %d\n", getpid(), getppid());
      process_reduce(start_index, end_index, ptr, *(&reduce_size[i]));
      exit(0);
    }
  }

  int n = num_reduces;
  for (i = 0; i < num_reduces; i++) {
    wait(NULL);
    printf("Ending Child Process ID: %d and Parent Process ID: %d\n", getpid(), getppid());
  }

  LinkedList * reduced_list = array_to_list(ptr);

  print_table(&reduced_list, 1);

  shm_unlink(data_array);


  printf("returns successfully?\n");
  return reduced_list;
}

void process_reduce(int start_index, int end_index, char * shared_memory, int num_words) {
  printf("Inside Process Reduce\n");
  if (num_words == 0) {
    return;
  }

  int words_compared = 0;

  int i = start_index;
  printf("Start Index: %d End Index: %d\n", start_index, end_index);
  printf("The number of words to handle is: %d\n", num_words);

  int current_block_size1 = bytes_to_int(shared_memory+i);
  printf("The current block_size: %d\n", current_block_size1);
  int count1 = bytes_to_int(shared_memory + i + 4);
  int count1_index = i + 4;
  printf("The count of this word is: %d\n", count1);
  int str_length1 = current_block_size1 - 8;
  int str1_index = i + current_block_size1 - str_length1;
  printf("The length of the string is: %d and it starts at index: %d\n", str_length1, str1_index);
  i += current_block_size1;
  num_words--;
  words_compared++;

  int unequal = 0;

  while (i < end_index && num_words > 0) {
    printf("\n\n");

    int current_block_size2 = bytes_to_int(shared_memory + i);
    printf("The current block size for string 2 is: %d\n", current_block_size2);
    int count2 = bytes_to_int(shared_memory + i + 4);
    int count2_index = i + 4;
    printf("The count of this word 2 is: %d\n", count2);
    int str_length2 = current_block_size2 - 8;
    int str2_index = i + current_block_size2 - str_length2;
    printf("The length of the string 2 is: %d and it starts at index: %d\n", str_length2, str2_index);
    i += current_block_size2;
    num_words --;
    words_compared++;
    printf("The value of words compared is: %d\n", words_compared);

    int j = 0;
    int k = 0;
    int index1 = str1_index;
    int index2 = str2_index;
    while (j < str_length1 && k < str_length2) {
      if (shared_memory[index1] != shared_memory[index2]) {
        printf("Unqual letter 1: %d   Unequal letter 2: %d\n", shared_memory[index1], shared_memory[index2]);
        unequal = 1;
        break;
      }
      j++;
      k++;
      index1++;
      index2++;
    }

    if (unequal == 1) {
      unequal = 0;
      printf("The two strings are unequal\n");
      if (num_words == 0) {
        printf("No more words left after the two strings are unequal\n");
        return;
      }
      current_block_size1 = current_block_size2;
      printf("The current block_size: %d\n", current_block_size1);
      count1 = count2;
      count1_index = count2_index;
      printf("The count of this word is: %d\n", count1);
      str_length1 = str_length2;
      str1_index = str2_index;
      printf("The length of the string is: %d and it starts at index: %d\n", str_length1, str1_index);
    }

    else if (unequal == 0) {
      printf("The two string are equal!\n");
      int temp_count = count1 + count2;
      printf("The merged counts are: %d\n", temp_count);
      shared_memory[count2_index] = temp_count;
      shared_memory[count1_index] = 0;

      current_block_size1 = current_block_size2;
      printf("The current block_size: %d\n", current_block_size1);
      count1 = count2;
      count1_index = count2_index;
      printf("The count of this word is: %d\n", count1);
      str_length1 = str_length2;
      str1_index = str2_index;
      printf("The length of the string is: %d and it starts at index: %d\n", str_length1, str1_index);

    }
    printf("\n\n\n\n");
  }
}

/* reduce function: wrapper for the reducer() function;
 * reduce spawns threads or processes of reducer()s and returns a LinkedList of the combined input
 */
LinkedList* reduce(LinkedList** reduce_table, int num_reduces, int process, int app, char *outfile) {
  int i;
  LinkedList* reduce_return = (LinkedList*) malloc(sizeof(LinkedList));

  if (process == 1) {
  //  run process code for reducers
    printf("We will use processess to reduce!\n");
    int * reduce_size = (int *) malloc(sizeof(int) * num_reduces);
    reduce_size = determineReduceSize(reduce_table, num_reduces);
    global_reduce_list = reduce_processes(reduce_table, reduce_size, num_reduces);
  } else {
    // run reducer() in a thread
    pthread_t* thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * num_reduces);
    for (i = 0; i < num_reduces; i++) {
      ReduceArgs* reduce_args = (ReduceArgs*) malloc(sizeof(ReduceArgs));
      reduce_args->list = reduce_table[i];
      reduce_args->app = app;

      pthread_create(&thread_ids[i], NULL, &reduce_thread_handler, (void*) reduce_args);
    }
    for (i = 0; i < num_reduces; i++) {
      LinkedList *reduced_return = (LinkedList *) malloc(sizeof(LinkedList));
      pthread_join(thread_ids[i], (void **) &reduced_return);

      pthread_mutex_lock(&list_mutex);
      global_reduce_list = concat_lists(global_reduce_list, reduced_return);
      pthread_mutex_unlock(&list_mutex);
    }
  }

  // After we get through the for loop, we know the global_reduce_list is
  // updated with the combines done by the reducers. However, we still
  // have to combine one more time to make sure that the same words in
  // different reducers still get merged together.

  if (app == 0) {
    pthread_mutex_lock(&list_mutex);
    global_reduce_list = combine(global_reduce_list);
    pthread_mutex_unlock(&list_mutex);
  }

  return global_reduce_list;
}

int main(int argc, char **argv) {

    printf("%d\n", argc);

    if (argc != 13) {
        fprintf(stderr, "Usage: %s --app [wordcount, sort] --impl [procs, threads] --maps num_maps --reduces num_reduces --input infile --output outfile\n", argv[0]);
        return 1;
    }

    //store command line arguments in variables
    char * program_type_leadin = argv[1];
    char * program_type = argv[2];

    char * parallel_type_leadin = argv[3];
    char * parallel_type = argv[4];

    char * num_maps_leadin = argv[5];
    int num_maps = atoi(argv[6]);

    char * num_reduces_leadin = argv[7];
    int num_reduces = atoi(argv[8]);

    char * input_file_path_leadin = argv[9];
    char * input_file_path = argv[10];

    char * output_file_path_leadin = argv[11];
    char * output_file_path = argv[12];


    //debugging statement for input arguments
    printf("%s %s %s %s %s %d %s %d %s %s %s %s\n", program_type_leadin, program_type, parallel_type_leadin, parallel_type, num_maps_leadin, num_maps, num_reduces_leadin, num_reduces, input_file_path_leadin, input_file_path, output_file_path_leadin, output_file_path);

    if (strcmp(program_type_leadin, "--app") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--app\": %s\n", program_type_leadin);
      return 1;
    }

    if (strcmp(parallel_type_leadin, "--impl") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--impl\": %s\n", parallel_type_leadin);
      return 1;
    }

    if (strcmp(num_maps_leadin, "--maps") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--maps\": %s\n", num_maps_leadin);
      return 1;
    }

    if (strcmp(num_reduces_leadin, "--reduces") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--reduces\": %s\n", num_reduces_leadin);
      return 1;
    }

    if (strcmp(input_file_path_leadin, "--input") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--input\": %s\n", input_file_path_leadin);
      return 1;
    }

    if (strcmp(output_file_path_leadin, "--output") != 0) {
      fprintf(stderr, "Invalid argument lead-in for \"--output\": %s\n", output_file_path_leadin);
      return 1;
    }

    int app;
    if (strcmp(program_type, "wordcount") == 0) {
        app = 0;
    } else if (strcmp(program_type, "sort") == 0) {
        app = 1;
    } else {
        fprintf(stderr, "Invalid program (--app) type (must be of [wordcount, sort])\n");
        return 1;
    }

    int processes;
    if (strcmp(parallel_type, "procs") == 0) {
        processes = 1;
    } else if (strcmp(parallel_type, "threads") == 0) {
        processes = 0;
    } else {
        fprintf(stderr, "Invalid parallel (--impl) type (must be of type [procs, threads])\n");
        return 1;
    }

    if (num_maps <= 0) {
      fprintf(stderr, "Invalid number of maps for --maps (must be an integer > 0): %s\n", argv[6]);
      return 1;
    }

    if (num_reduces <= 0) {
      fprintf(stderr, "Invalid number of reduces for --reduces (must be an integer > 0): %s\n", argv[8]);
      return 1;
    }

    FILE* test_input = fopen(input_file_path, "r");
    if (test_input == NULL) {
      fprintf(stderr, "Invalid input file path (--input): %s\n", input_file_path);
      return 1;
    }
    fclose(test_input);

    if (num_reduces <= 0) {
      fprintf(stderr, "Invalid number of reduces for --reduces (must be an integer > 0): %s\n", argv[8]);
      return 1;
    }

    global_map_list = map(input_file_path, processes, num_maps);

    LinkedList **reduce_table = build_reduce(global_map_list, num_reduces, app);

    printf("Printing reduce table...\n");
    //print_table(reduce_table, num_reduces);
    printf("Creating global_reduce_list...\n");

    global_reduce_list = reduce(reduce_table, num_reduces, processes, app, output_file_path);
    printf("Made it to main!\n");


    printf("Global reduce list size: %d\n", global_reduce_list->size);
   // traverse(global_reduce_list, 1);

    output_list(global_reduce_list, output_file_path, app);

    return 0;
}
