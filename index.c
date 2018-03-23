#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> //TODO see if this is necessary
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

int printList();

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
  printf("Attempting to insert %s from file %s\n", s, f);
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
    //printf("comparing %s and %s\n", new_big->word, tmp1->word);

    /* case where new_node's string is less than tmp1's string in terms of alphabetic precedence */
    if (strcmp(new_big->word, tmp1->word) <= 0) {
      //printf("%s is less than %s\n", new_big->word, tmp1->word);
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
    printf("%s %d\n", tmp1->word, big_head->little_head->count);
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

/* function to find whether or not a given character is alphabetic (returns 1), a number (returns 2), or not valid (returns 0) */
int isAlphanumeric(char c) {

  /* checks to see if the character is alphabetic (both upper and lowercases) */
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
    return 1;
  }
  else if (c >= '0' && c <= '9') {
    return 2;
  }

  //  c is non-alphanumeric
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

/* function to find if the input string has a forbidden name for a directory (i.e. do not try to open this) and if so, returns 0 */
int isValid(char* str) {
  if (strcmp(str, ".") == 0 || strcmp(str, "..") == 0 || strcmp(str, ".git") == 0 || strcmp(str, ".DS_store") == 0) {
    printf("Found %s to have an invalid name\n", str);
    return 0;
  }

  return 1;
}

/* function to determine if the input string is the name of a valid directory. returns -2 if yes, returns a file descriptor if input string is suspected to be a file, returns -1 if invalid or nonexistent */
int isDirectory(char* str) {
//  printf("Checking to see if %s is a directory or not\n", str);

  // effectively removes the trailing forward slash, if it exists in the input statement
  if (str[len(str) - 1] == '/') {
    str[len(str) - 1] = '\0';
  }

//  printf("Removed possible trailing forward slash; it has become %s\n", str);

/* commented out becausse this is redundant TODO make sure this is actually unnecessary
  // finds if the input string has a forbidden name (i.e. do not try to open this) and if so, returns -1
  if (!isValid(str)) {
    return -1;
  }
*/

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
int processFile(int fd, char * file_name) {
  printf("File: %s passed in with descriptor %d\n", file_name, fd);
  char * buffer = (char *) calloc(101, sizeof(char)); //buffer that will hold 100 characters at a time. allocated 101 because of 1 extra character for the null terminator
  printf("String to be tokenized: %s\n", buffer);
  int buf_length = 100; //length of the string from the buffer
  int i = 0; //this will be used to traverse the string from the buffer
  int j = 0; //this will be used to traverse the string for each token
  printf("The name of the file is: %s\n", file_name);
  int start = 0; //integer to tell whether we are starting a new token. 0 if false, 1 if true.
  int tokenized = 0; //integer to tell whether a string has been successfully tokenized. 0 if false, 1 if true.
  int incomplete = 0; //integer to tell if a token was cut off due to the limited size of the bugger. 0 if false, 1 if true
  char * partial_str = (char*) malloc (2*(buf_length - i + 1) * sizeof(char));
  int bytesread = 0; //number of bytes that read has read (corner case for the end of the file)

  /* loop to traverse through the entire file */
  while ((bytesread = read(fd, buffer, 100)) != 0) {
    printf("The string to be tokenized: %s\n", buffer);
    //  scan through the big string and construct the linked list
    for (i = 0; i < buf_length && i < bytesread - 1; i++) {
      char * token = (char*) malloc ((buf_length - i + 1) * sizeof(char)); // this weird allocation was made to account for the biggest possible length of the substring at this point in the program, since we don't know how long each substring is beforehand
      tokenized = 0; //reset tokenized to 0 because we are creating the next token
      j = 0; //reset j to 0 so that the new token starts at index 0 rather than i
      start = 1; //we are starting a new token so set start to 1
      if (incomplete == 1) {
        /*if the start of the next string to be tokenized has the rest of the partial string */
        if (isAlphanumeric(buffer[i]) == 1 || isAlphanumeric(buffer[i]) == 2) {
          printf("The character at buffer of i is %c with index %d\n", buffer[i], i);
          int temp = strlen(partial_str);
          printf("The length of the partial string is %d\n", temp);
          while (isAlphanumeric(buffer[i]) && i < buf_length) {
            partial_str[temp] = tolower(buffer[i]);
            i++;
            temp++;
          }
          partial_str[temp] = '\0';
          printf("Incomplete completed successully. String is %s\n", partial_str);
          incomplete = 0;
          tokenized = 0;
          insert(partial_str, file_name);
        }
        /* if the first character was a delimiter */
        else if (isAlphanumeric(buffer[i]) == 0) {
          printf("The first character of the buffer was a delimiter\n");
          insert(partial_str, file_name);
          incomplete = 0;
        }
      }

      /* loop that eliminates any leading digits */
      while (isAlphanumeric(buffer[i]) == 2 && start == 1 && i < buf_length) {
        i++;
      }
      /* once we finish removing all the leading numbers lets get the actual token */
      if (isAlphanumeric(buffer[i]) == 1) {
        start = 0;
      }
      /* checks that each character in the string is alphabetic. if a character is not alphabetic, then it is a delimiter */
      while (isAlphanumeric(buffer[i]) && start == 0 && i < buf_length) {
        token[j] = tolower(buffer[i]);
  //      printf("big_str[%d] is %c, str[%d] is %c\n", i, big_str[i], j, str[j]);
        i++;
        j++;
        tokenized = 1;
      }
      /*check to see that we are at the end of the string. if so, then copy the current token*/
      if (i == buf_length && (isAlphanumeric(buffer[buf_length - 1]) == 1 || isAlphanumeric(buffer[buf_length - 1]) == 2)) {
        int k = 0;
        token[j] = '\0';
        printf("Token to be partial: %s\n", token);
        for (k = 0; k < strlen(token); k++) {
          partial_str[k] = token[k];
        }
        partial_str[k] = '\0';
        printf("partial %s\n", partial_str);
        incomplete = 1;
      }
      /* if enough memory was alloted for the string then we can insert it into the linked list */
      else if (tokenized == 1) {
        token[j] = '\0';
        insert(token, file_name);
        //printf("insert complete. printing current list...\n");
      }

    }
  }
  printList();
  return 0;
}

int processDir(char* dir_name) {

//    printf("%s entered processDir\n", dir_name);
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

      // check to see if the directory has been exhausted, and if so, returns from the method
      if (dp == NULL) {
        printf("Directory %s processed!\n", dir_name);
        return 0;
      }

      printf("%s found in processDir\n", dp->d_name);

        dir_name_len = len(dir_name); // length of the input path name (not including a trailing forward slash)
        d_name_len = len(dp->d_name); // length of the new file's/directory's name (not including its whole path)
        new_dir_len = dir_name_len + d_name_len + 2; // the +2 adds space for the newest forward slash, placed between the old path and the new file/directory, as well as the closing '\0'
//        printf("the length of the full path for %s is %d\n", dp->d_name, new_dir_len);
        char* new_dir = (char*)malloc(sizeof(char) * new_dir_len);
        new_dir[new_dir_len - 1] = '\0';

        int j = 0;
        for (i = 0; i < new_dir_len; i++) {
          if (i < dir_name_len) {
            new_dir[i] = dir_name[i];
          } else if (i == dir_name_len) {
            new_dir[i] = '/';
          } else if (i == new_dir_len - 1) {
            new_dir[i] = '\0';
          } else {
            new_dir[i] = dp->d_name[j];
            j++;
          }
        }

      printf("Its full path is %s\n", new_dir);
      dircision = isDirectory(new_dir);

      if (!isValid(dp->d_name) && dircision < 0) {
        // dp is invalid
        printf("Caution! Requested input %s is not a valid file or directory. Continuing...\n", new_dir);
        continue;
      }

      if (isValid(dp->d_name) && dircision == -2) {
        // dp is a directory

        printf("File path for %s from starting directory is %s\n", dp->d_name, new_dir);

        processDir(new_dir);
        free(new_dir);
      } else if (dircision >= 0) {
        // dp is probably a file
        processFile(dircision, dp->d_name);
      } else {
        // not good
        printf("??????????????????????????\n");
      }

    } while (1);

  return 0;
}

/* main */
int main(int argc, char** argv) {
  printf("\n");

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
//  printf("fd returns as %d\n", fd);
//  printf("errno returns as %d\n", errno);

  // checks to see if input can be opened as a file or directory at all
  if (fd == -1) {
    // input is not a directory; exit on error
    printf("Error! Requested input %s is not a valid file or directory. Exiting...\n", argv[2]);
    exit(0);
  }

  fd = isDirectory(argv[2]);
  printf("%d is fd\n", fd);

  // checks to see if input is valid
  if (!isValid(argv[2])) {
    // input is a forbidden value; exit on error
      printf("Error reading %s as the command line argument! Forbidden name...\n", argv[2]);
      exit(0);
  }

  // checks to see if input is a directory
  if (fd == -2) {
    // input is a directory, so treat it as one
      printf("Reading %s as a directory...\n", argv[2]);
      processDir(argv[2]);
  } else if (fd >= 0) {
    // input is not a directory, so treat it as a single file
      char * file_name = (char *) malloc (sizeof(char) * 262);
      int i = 0; //traverse argv[2]
      int j = 0; //traverse the file_name
      int length = 0; //length of the file name
      int file_path_len = strlen(argv[2]);
      for (i = file_path_len - 1; i >= 0; i--) {
        if (argv[2][i] == '/') {
          break;
        }
        length++;
      }
      printf("The length of the file is: %d\n", length);
      for (i = file_path_len - length; i < file_path_len; i++) {
        file_name[j] = argv[2][i];
        j++;
      }
      file_name[j] = '\0';
      printf("Reading %s as a file...\n", file_name);
      processFile(fd, file_name);
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

[ ] are file descriptors with values greater than 3 when you open normal?

[ ] size of the buffer

   types of input to account for:

[ ] file only

[ ] directory only

[ ] directory leading to file

[ ] directory leading to another directory

[ ] directory leading to another directory leading to a file

[ ] invalid input types

[ ] a file or directory that the user does not have permission to access (?)

*/
