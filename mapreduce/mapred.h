#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsers.h"

#ifndef MAPRED_H
#define MAPRED_H


typedef struct _TpTable {
  char * word;
  int count;
  struct _TpTable *next;
} TpTable;


/*
 * Wrapper function for both the map_processes() and map_threads()
 * functions.
 * First, parse through the input file. Then, build a TpTable array
 * based on the number of maps we need to do. Then, send those off
 * to either processes (if processes == 1) or threads (if
 * processes == 0).
 * Return the result of the mapping operation.
 */
LinkedList *map(char *filename, int processes, int num_maps);




void *map_thread_handler(void *args);

LinkedList *tpTable_to_list(TpTable *table);

/*
 * Mapper function with threads.
 * Just sets counts to 1 and puts the maps back together.
 */
LinkedList *map_threads(TpTable **hashmap, int num_maps);

LinkedList *map_processes(TpTable ** hashmap, LinkedList * list, int num_maps, int array_size, int * map_size);

void process_map(int start_index, int end_index, unsigned char * sharedmemory);

int * determineMapSize(int num_words, int num_maps);

int * startEnd (TpTable ** map, LinkedList ** reduce_table, int map_or_reduce, unsigned char * sharedMemory, int array_length, int * map_size, int num_maps);

LinkedList * reduce_processes(LinkedList ** reduce_table, int * reduce_size, int num_reduces);

void process_reduce(int start_index, int end_index, unsigned char * shared_memory, int num_words);

/* Utility function that creates nodes to be placed inside the hashmap */
TpTable * createHashMapNode(char * word, int count);

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap(LinkedList * list, int num_maps, int * map_size);

/* Utility function to print out the global hashmap for debugging purposes */
void printmap(TpTable ** map, int num_maps);

void* reduce_thread_handler(void* reduce_args);

LinkedList* reduce(LinkedList** reduce_table, int num_reduces, int process, int app, char *outfile);

#endif
