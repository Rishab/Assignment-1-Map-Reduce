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

LinkedList * map(TpTable ** hashmap, LinkedList * list, int num_maps, int array_size, int * map_size);

void process_map(int start_index, int end_index, char * sharedmemory);

int * determineMapSize(int num_words, int num_maps);

int * startEnd (TpTable ** map, char * sharedMemory, int array_length, int * map_size, int num_maps);

/* Utility function that creates nodes to be placed inside the hashmap */
TpTable * createHashMapNode(char * word, int count);

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap(LinkedList * list, int num_maps, int * map_size);

/* Utility function to print out the global hashmap for debugging purposes */
void printmap(TpTable ** map, int num_maps);

void* reducer(void* reduce_args);

LinkedList* reduce(LinkedList** reduce_table, int num_reduces, int process);

#endif
