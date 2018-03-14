#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * invertedIndex.c
 * by Jason Scot (Section 03) and Rishab Chawla (Section 02)
 * for Systems Programming with Prof. John-Austen Francisco
 * Rutgers University
 * 03/25/2018
 *
 */

/* definition of nodes in the "little" (i.e. file-focused) linked lists */
typedef struct _LittleNode {
  char* file_name;
  int count;
  struct _LittleNode* next;
} Node;

/* definition of nodes in the "big" (i.e. word-focused) linked list */
typedef struct _BigNode {
  char* word;
  LittleNode* litte_head;
  struct _BigNode* next;
} BigNode;

/* global declaration of head of the "big" list so it can be referenced later */
BigNode *big_head = NULL;

/* function to insert a new node into the 2D list at the correct spot; CAUTION: the input parameter must be a proper string (i.e. ending in '\0')! */
int insert(char* s, char* f) {

  /* if the head is NULL, then the linked list is empty so this inserts a node at the head */
  if (big_head == NULL) {
    BigNode* new_big = (BigNode*) malloc(sizeof(BigNode));
    new_big->word = s;
    LittleNode* new_little = (LittleNode*) malloc(sizeof(LittleNode));
    new_little->count = 1;
    new_little->file_name = f;
    new_little->next = NULL;
    new_big->little_head = new_little;
    new_big->next = NULL;
    big_head = new_big
    return 0;
  }

  /* checks whether or not the word being insterted already exists in the big list */
  BigNode* tmp1 = big_head; //searching pointer
  while (tmp1 != NULL) {
    if (strcmp(s, tmp1->word) == 0) {
        /* the word does exist already; see whether or not its file exists yet */
        LittleNode* tmp = tmp1->little_head; //searching pointer
        while (tmp->next != NULL) {
          if (strcmp(f, tmp->file_name) == 0) {
            /* file does exist, so increment file count */
            tmp->count++;
            return 0;
          }

          tmp = tmp->next;
        }

        if (strcmp(f, tmp->file_name) == 0) {
          /* file does exist, so increment file count */
          tmp->count++;
          return 0;
        }

        /* file does not exist, so make a new file node for it */
        LittleNode* new_little = (LittleNode*) malloc(sizeof(LittleNode));
        new_little->count = 1;
        new_little->file_name = f;
        new_little->next = NULL;
        tmp->next = new_little;
    }

    tmp1 = tmp1->next;
  }

  /* the word does not exist yet, so makes a new node for it */
  
  BigNode* new_big = (BigNode*) malloc(sizeof(BigNode));
  new_big->word = s;
  LittleNode* new_little = (LittleNode*) malloc(sizeof(LittleNode));
  new_little->count = 1;
  new_little->file_name = f;
  new_little->next = NULL;
  new_big->little_head = new_little;

  /*tmp1 and tmp0 are pointers to traverse the linked list while inserting*/
  tmp1 = head; //leading pointer
  BigNode* tmp0 = head; //trailing pointer

  /* while loop to go to the end of the linked list in the new node is greater than all of the nodes*/
  while (tmp1 != NULL) {
//    printf("comparing %s and %s\n", new_node->str, tmp1->str);

    /* case where new_node's string is less than tmp1's string in terms of alphabetic precedence */
    if (strcmp(new_big->word, tmp1->word) <= 0) {
//      printf("%s is less than %s\n", new_node->str, tmp1->str);
      new_big->next = tmp1;

      /* case that the node should be inserted to the front of the list */
      if (tmp0 == tmp1) {
        head = new_big;
      }
      /* case that the node should be inserted in the middle of the list */
      else {
        tmp0->next = new_big;
      }
      return 0;
    }

    /* Continue iterating down the list until: 1. new node's string is less than tmp1's string or
    2. the end of the linked list has been reached */

//    printf("%s is greater than %s; iterating down\n", new_node->str, tmp1->str);
    /* case that the node should be inserted at the end of the list */
    if (tmp1->next == NULL) {
//      printf("inserting %s at the end of the list\n", new_node->str);
      tmp1->next = new_big;
      new_big->next = NULL;
      return 0;
    }
    /* make tmp1 point to the next link and keep tmp0 at the current link */
    tmp0 = tmp1;
    tmp1 = tmp1->next;
  }

  tmp1->next = head;
  head = tmp1;
  return 0;
}

/* diagnostic function to write the list to standard output with each string starting a new line */
int printList() {
  /* pointer to beginning of the list */
  BigNode* tmp1 = big_head;

  /* loop to iterate through the big list */
  while (tmp1 != NULL) {
    printf("%s\n", tmp1->word);
    LittleNode* tmp2 = big_head;
    /* loop to iterate through the little list */
    while (tmp2 != NULL) {
      printf("%s: %d", tmp2->file_name, tmp2->count);
      tmp2 = tmp2->next;
    }

    tmp1 = tmp1->next;
  }
  return 0;
}

/* function to free the list so that the program can be run multiple times without
without any memory leaks */
int freeList() {
  /* pointers used to free the list */
  struct node* tmp0; //leading pointer
  struct node* tmp1 = big_head; //trailing pointer

  /* Loop to iterate through the list. It frees the current node's fields (including the list it points to), then moves to the next node, and so on */
  while (tmp1 != NULL){
    tmp0 = tmp1->next;

    LittleNode* tmp2; //leading pointer
    LittleNode* tmp3 = tmp1->little_head; //trailing pointer
    while (tmp3 != NULL){
      tmp2 = tmp3->next;
      free(tmp3->file_name);
      free(tmp3->count);
      free(tmp3);
      tmp3 = tmp2;
    }

    free(tmp1->word);
    //TODO: decide if we also have to free tmp1->little_head here, or if that was taken care of already
    free(tmp1);
    tmp1 = tmp0;
  }
  return 0;
}

/* function to find whether or not a given character is alphabetic */
int isAlphabetic(char c) {

  /* checks to see if the character is alphabetic (both upper and lowercases) */
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
    return 1;
  }

  //  c is non-alphabetic
  return 0;
}

/* function to find length of a string. careful, it can loop infinitely if you do it on an invalid string or after the \0 character in the array */
int len(char* str) {
  int l = 0;

  while (str[l] != '\0') {
    l++;
  }

  return l;
}

/* main */
int main(int argc, char** argv) {

  /* check that the input has one and only one argument, else exit with error */
  if (argc != 2) {
    printf("Error! Program must be run with exactly one argument, but %d were found. Exiting...\n", argc);
    exit(0);
  }

  return 0;
}


/* things TODO:
  
[ ] make file reader that creates a buffer and tokenizes each word, converting each character to lowercase

[ ] make sure that each input to insert() is a proper string! (ends in '\0')

[ ] feed each token to insertBig()

[ ] sort each little list by its occurrence number at the end of the program

*/
