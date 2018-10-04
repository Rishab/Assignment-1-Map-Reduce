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

/* Utility function that creates nodes to be placed inside the hashmap */
TpTable * createHashMapNode(char *word);

/* Utility function to create a hashmap with all the nodes that each thread or process will manage.
   This is to ensure that when map combines nodes together, the grouping isn't lost between processes
   and threads so that everything stays consistent.
*/
void fillHashMap();

/* Utility function to print out the global hashmap for debugging purposes */
void printmap();


#endif