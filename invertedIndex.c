#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> //TODO see if this is necessary
#include <dirent.h>
#include <errno.h>

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
} LittleNode;

/* definition of nodes in the "big" (i.e. word-focused) linked list */
typedef struct _BigNode {
  char* word;
  LittleNode* little_head;
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
    big_head = new_big;
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
  tmp1 = big_head; //leading pointer
  BigNode* tmp0 = big_head; //trailing pointer

  /* while loop to go to the end of the linked list in the new node is greater than all of the nodes*/
  while (tmp1 != NULL) {
//    printf("comparing %s and %s\n", new_node->str, tmp1->str);

    /* case where new_node's string is less than tmp1's string in terms of alphabetic precedence */
    if (strcmp(new_big->word, tmp1->word) <= 0) {
//      printf("%s is less than %s\n", new_node->str, tmp1->str);
      new_big->next = tmp1;

      /* case that the node should be inserted to the front of the list */
      if (tmp0 == tmp1) {
        big_head = new_big;
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

  tmp1->next = big_head;
  big_head = tmp1;
  return 0;
}

/* diagnostic function to write the list to standard output with each string starting a new line */
int printList() {
  /* pointer to beginning of the list */
  BigNode* tmp1 = big_head;

  /* loop to iterate through the big list */
  while (tmp1 != NULL) {
    printf("%s\n", tmp1->word);
    LittleNode* tmp2 = big_head->little_head;
    /* loop to iterate through the little list */
    while (tmp2 != NULL) {
      printf("  %s: %d\n", tmp2->file_name, tmp2->count);
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
  BigNode* tmp0; //leading pointer
  BigNode* tmp1 = big_head; //trailing pointer

  /* Loop to iterate through the list. It frees the current node's fields (including the list it points to), then moves to the next node, and so on */
  while (tmp1 != NULL){
    tmp0 = tmp1->next;

    LittleNode* tmp2; //leading pointer
    LittleNode* tmp3 = tmp1->little_head; //trailing pointer
    while (tmp3 != NULL){
      tmp2 = tmp3->next;
      free(tmp3->file_name);
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

/* function to determine if the input string is the name of a valid directory. returns -2 if yes, returns a file descriptor if input string is suspected to be a file, returns -1 if invalid or nonexistent */
int isDirectory(char* str) {

  // effectively removes the trailing forward slash, if it exists in the input statement
  if (str[len(str) - 1] == '/') {
    str[len(str) - 1] = '\0';
  }

  // finds if the input string has a forbidden name (i.e. do not try to open this) and if so, returns -1
  if (strcmp(str, ".") == 0 || strcmp(str, "..") == 0 || strcmp(str, ".git") == 0 || strcmp(str, ".DS_store") == 0) {
    printf("Found %s to have an invalid name\n", str);
    return -1;
  }
  
  // opens the string in directory-only mode
  int fd = open(str, O_DIRECTORY);
  // checks to see if input is a directory
  if (fd != -1) {
    // input is a directory, so return 1
      printf("Found %s is a directory\n", str);
      return -2;
  }

  fd = open(str, O_RDONLY);
  // input is not a directory (probably a file), return its file descriptor
  printf("Found %s is a file; its fd is %d\n", str, fd);
  return fd;
}

/* processes an already-opened file into the linked list, given its file descriptor */
int processFile(int fd) {
  
  printf("File processed!\n");
  return 0;
}

int processDir(char* dir_name) {

    DIR* dir;
    struct dirent *dp;
    dir = opendir (dir_name);
    if (dir == NULL) {
      perror ("Caution! Directory %s could not be opened; continuing...");
      return 0;
    }

    int dircision;
    int i, new_dir_len, dir_name_len, d_name_len;

    do {
      dp = readdir(dir);
      dircision = isDirectory(dp->d_name);

      if (dircision == -1) {
        // dp is invalid
        printf("Caution! Requested input (file or directory found in %s) is not a valid file or directory. Continuing...\n", dir_name);
        return 0;
      }

      if (dircision == -2) {
        // dp is a directory

        dir_name_len = len(dir_name); // length of the input path name (not including a trailing forward slash)
        d_name_len = len(dp->d_name); // length of the new file's name (not including its whole path)
        new_dir_len = dir_name_len + d_name_len + 1;
        char* new_dir = (char*)malloc(sizeof(char) * new_dir_len);
        new_dir[new_dir_len - 1] = '\0';

        for (i = 0; i < new_dir_len; i++) {
          if (i < dir_name_len) {
            new_dir[i] = dir_name[i];
          } else if (i == dir_name_len) {
            new_dir[i] = '/';
          } else if (i == new_dir_len - 1) {
            new_dir[i] = '\0';
          } else {
            new_dir[i] = dp->d_name[i];
          }
        }
      
        printf("File path for %s from starting directory is %s\n", dp->d_name, new_dir);

        processDir(new_dir);
        free(new_dir);
      } else if (dircision >= 0) {
        // dp is probably a file
        processFile(dircision);
      } else {
        // not good
        printf("??????????????????????????\n");
      }

    } while (dp != NULL);

  return 0;
}

/* main */
int main(int argc, char** argv) {

  /* check that the input has two and only two arguments, else exit with error */
  if (argc != 3) {
    printf("Error! Program must be run with exactly two arguments, but %d were found. Exiting...\n", argc - 1);
    exit(0);
  }

  // effectively removes the trailing forward slash, if it exists in the input statement
  if (argv[2][len(argv[2]) - 1] == '/') {
    argv[2][len(argv[2]) - 1] = '\0';
  }

  printf("Output will be stored in an XML file with the following name: %s\n", argv[1]);
  printf("Reading from input with the following name: %s\n", argv[2]);
  
  int fd = open(argv[2], O_RDONLY);
  printf("fd returns as %d\n", fd);
  printf("errno returns as %d\n", errno);  

  // checks to see if input can be opened as a file or directory at all
  if (fd == -1) {
    // input is not a directory; exit on error
    printf("Error! Requested input %s is not a valid file or directory. Exiting...\n", argv[2]);
    exit(0);
  }

  fd = isDirectory(argv[2]);
  printf("%d is fd\n", fd);
  
  // checks to see if input is a directory
  if (fd == -2) {
    // input is a directory, so treat it as one
      printf("Reading %s as a directory...\n", argv[2]);
      processDir(argv[2]);
  } else if (fd == -1) {
    // input is a forbidden value; exit on error
      printf("Error reading %s as the command line argument! Forbidden name...\n", argv[2]);
      exit(0);
  } else if (fd >= 0) {
    // input is not a directory, so treat it as a single file
      printf("Reading %s as a file...\n", argv[2]);
      processFile(fd);
  } else {
    // somehow this was reached, and that means there is a bad error in isDirectory
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      exit(0);
  }
 
  return 0;
}


/* things TODO:
  
[ ] make file reader that creates a buffer and tokenizes each word, converting each character to lowercase

[ ] make sure that each input to insert() is a proper string! (ends in '\0')

[ ] feed each token to insertBig()

[ ] sort each little list by its occurrence number at the end of the program

[ ] in processFile, make sure the file is of a legal type to be processed

[ ] in processDir, ignore .git and .DS_store and . and ..

[ ] process list into XML format


   types of input to account for:

[ ] file only

[ ] directory only

[ ] directory leading to file

[ ] directory leading to another directory

[ ] directory leading to another directory leading to a file

[ ] invalid input types

[ ] a file or directory that the user does not have permission to access (?)

*/
