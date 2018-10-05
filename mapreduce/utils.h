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


LinkedList** build_reduce(LinkedList** map_table, int num_maps, int num_reduces);

int bytes_to_int(unsigned char *c);

unsigned char *list_to_array(LinkedList *list);

LinkedList *array_to_list(unsigned char *arr);


#endif