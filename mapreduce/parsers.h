/*
 * parsers.h
 * Code for everything related to parsing the input files.
 */

#include "structures.h"

#ifndef PARSERS_H
#define PARSERS_H

#define WC_BUF_SIZE     100    // Size of buffer read into for word count 
#define SORT_BUF_SIZE    20    // Size of buffer read into for sorting


/*
* Parses an input file, and constructs a WCLinkedList struct of the words
* in the file.
*/
LinkedList *word_count_parse(char *file);


/*
 * Concatenates the partial and buffer strings into one string.
 * Note: 'partial' is dynamic memory, 'buffer' is on the stack.
 *       this function frees the memory that partial is in.
 */
char *merge_tokens(char *partial, char *buffer);


#endif
