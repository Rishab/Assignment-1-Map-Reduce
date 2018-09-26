/*
 * parsers.h
 * Code for everything related to parsing the input files.
 */

#ifndef PARSERS_H
#define PARSERS_H

#define WC_BUF_SIZE     100    // Size of buffer read into for word count 
#define SORT_BUF_SIZE    20    // Size of buffer read into for sorting

/*
 * Node of a linked list, if we're sorting integers.
 * Note: the 'word' field should be malloc'ed.
 */
typedef struct _WCParseNode {
    char *word;
    struct _WCParseNode *next;
} WCParseNode;

WCParseNode *WCParseNode_new(char *word, WCParseNode *next);


/*
 * Container for a linked list if we're counting words.
 * Stores the size of the linked list and the head.
 */
typedef struct _WCLinkedList {
    int size;
    WCParseNode *head;
} WCLinkedList;


WCLinkedList *WCLinkedList_new();

void print_wc_list(WCLinkedList *list);


/*
 * Node of a linked list, if we're sorting integers.
 */
typedef struct _SParseNode {
    int num;
    struct _SParseNode *next;
} SParseNode;


/*
 * Container for a linked list if we're sorting numbers.
 * Stores the size of the linked list and the head.
 */
typedef struct _SLinkedList {
    int size;
    SParseNode *head;
} SLinkedList;


/*
* Parses an input file, and constructs a WCLinkedList struct of the words
* in the file.
*/
WCLinkedList *word_count_parse(char *file);

SLinkedList *sort_parse(char *file);

/*
 * Concatenates the partial and buffer strings into one string.
 * Note: 'partial' is dynamic memory, 'buffer' is on the stack.
 *       this function frees the memory that partial is in.
 */
char *merge_tokens(char *partial, char *buffer);


#endif
