
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
    traverse(global_map_list, 1);
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
  /*
  //setup variables for out shared memory region
  printf("Our shared memory region will be %d bytes long.\n", array_size);

  //key_t shm_key = 706286;

  int shm_id = shmget(101, array_size, 0666 | IPC_CREAT);
  if (shm_id < 0) {
    printf("shmget failed to return a shared memory region");
    return;
  }

  data_array = (char *) shmat(shm_id, (char*) 0, 0);
  if (data_array == (void*)-1) {
    printf("shmat failed to attach a shared memory region");
  }

  printf("1. Our shared memory region is attached at the following address: %p\n", data_array);

  char * temp = list_to_array(list);
  memcpy(data_array, temp, array_length);
  free(temp);

  printf("\n");
  //print out the contents of the array for testing purposes
  int i;
  for (i = 0; i < array_length; i++) {
  	printf("%d  ", data_array[i]);
  }
  printf("\n");

  printf("2. Our shared memory region is attached at the following address: %p\n", data_array);
  */

  int sharedmem_fd = shm_open(mem_name, O_RDWR | O_CREAT, 0666);

  if (sharedmem_fd < 0) {
    printf("This is fucked\n");
  }
  char * temp = list_to_array(list);
  int array_length2 = bytes_to_int(temp);

  ftruncate(sharedmem_fd, array_length2);

  char * ptr = (char *) mmap(&data_array, array_length2, PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem_fd, 0);


  memcpy(ptr, temp, array_length2);
  print_memory(ptr, array_length2);
  printf("Our shared memory region is attached at the following address: %p\n", ptr);

  printf("Before startEnd function call\n");
  int * start_end = startEnd(hashmap, ptr, array_length2, map_size, num_maps);

  pid_t process_ids[num_maps];

  int i;
  for (i = 0; i < num_maps; i++) {
    process_ids[i] = fork();
    int start_index = start_end[2*i];
    int end_index = start_end[2*i+1];
    if (process_ids < 0) {
      printf("Fork failed");
      abort();
    }
    else if (process_ids[i] == 0) {
      printf("\nFork spawned a process sucessfully!\n");
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

int * startEnd (TpTable ** map, char * sharedMemory, int array_length, int * map_size, int num_maps) {
  printf("Inside StartEnd function");
  int * start_end = (int *) malloc(sizeof(int) * 2 * num_maps);
  int i;
  int j = 8; //keeps track of the current index in the sharedMemory array
  int tracker = 0; //keeps track of which process/thread maps to which indexes in the returned array

  int shm_length = bytes_to_int(sharedMemory);

  printf("SHM length: %d\n", shm_length);
  //traverse the entire map and sharedMemory to determine which processes map to which indexes in the array
  for (i = 0; i < num_maps; i++) {
    int current_num_words = map_size[i];
    printf("The current number of words to discover the chunk for are: %d\n", current_num_words);
    int start = j;
    int end = j;
    //printf("The value of start is %d and end is %d\n", start, end);
    while (j < array_length && current_num_words != 0) {
      int current_block_size = bytes_to_int(sharedMemory + j);
      printf("The size of the current block is: %d\n", current_block_size);
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
  for (i = 0; i < 2 * num_maps; i++) {
    printf("%d\t", start_end[i]);
  }
  printf("\n");

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

/* reducer function (used by reduce): given list gets assigned to a process or thread to perform the reduce operation,
 * that is, for all the words the list's counts get combined
 * Whether there is a thread or process required, the list will run the algorithm
 * using multiple processes or threads
 */
void* reduce_thread_handler(void* reduce_args) {
  ReduceArgs *args = (ReduceArgs *) reduce_args;
  LinkedList* list = args->list;
  int process = args->process;

  int i;
  int synonyms;

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
/*
  pthread_mutex_lock(&list_mutex);
  global_reduce_list = concat_lists(global_reduce_list, reduced_list);
  pthread_mutex_unlock(&list_mutex);
*/
  pthread_exit((void *) reduced_list);
  return (void *) reduced_list;
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
    pthread_t* thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * num_reduces);
    for (i = 0; i < num_reduces; i++) {
      ReduceArgs* reduce_args = (ReduceArgs*) malloc(sizeof(ReduceArgs));
      reduce_args->list = reduce_table[i];
      reduce_args->process = process;
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
  pthread_mutex_lock(&list_mutex);
  global_reduce_list = combine(global_reduce_list);
  pthread_mutex_unlock(&list_mutex);

  return global_reduce_list;
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
    
    if (argc != 7) {
        fprintf(stderr, "Usage: %s [wordcount, sort] [processes, threads] num_maps num_reduces input output\n", argv[0]);
        return 1;
    }

    int app;
    if (strcmp(program_type, "wordcount") == 0) {
        app = 0;
    } else if (strcmp(program_type, "sort") == 0) {
        app = 1;
    } else {
        fprintf(stderr, "Invalid program type.\n");
        return 1;
    }

    int processes;
    if (strcmp(parallel_type, "processes") == 0) {
        processes = 1;
    } else if (strcmp(parallel_type, "threads") == 0) {
        processes = 0;
    } else {
        fprintf(stderr, "Invalid parallel type.\n");
        return 1;
    }
   
    global_map_list = map(input_file_path, processes, num_maps);

    LinkedList **reduce_table = build_reduce(global_map_list, num_reduces);
    
    print_table(reduce_table, num_reduces);

    global_reduce_list = reduce(reduce_table, num_reduces, processes);


    traverse(global_reduce_list, 1);
    return 0;
}

/*

Things TO DO:

[x] edit fillHashMap function and call that before call to map function

[x] figure out how to split memory into chunks

[x] create a spawn process function to spawn processes inside of map_

[x] change the value of count in the map function for each and every array

[x] convert array to linked list and return it from map

*/
