/*
 * utils.h
 * Miscellaneous functions that don't really need their own separate headers.
 */

#include "structures.h"

#ifndef UTILS_H
#define UTILS_H

char **split(char *file, int m);

void print_array(char **arr, int len);

int output_list(LinkedList* list, char* filename, int numbers);

int comp(const void *x, const void *y);

LinkedList** build_reduce(LinkedList* mapped_list, int num_reduces, int app);

void print_memory(unsigned char *array, int size);

void int_to_bytes(int i, unsigned char *c);

int bytes_to_int(unsigned char *c);

unsigned char *list_to_array(LinkedList *list);

unsigned char * table_to_array(LinkedList ** reduce_table, int * reduce_size, int num_reduces); 

LinkedList *array_to_list(unsigned char *arr);

LinkedList *combine(LinkedList *list);

#endif
