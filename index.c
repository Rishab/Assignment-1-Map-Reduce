#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> //TODO see if this is necessary
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

/*
 * index.c
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
  int little_size; //the amount of nodes in the little list
  LittleNode* little_head;
  struct _BigNode* next;
} BigNode;

/* global declaration of head of the "big" list so it can be referenced later */
BigNode *big_head = NULL;

/* custom, simpler strcmp method to compare two strings and place them in the order described for the assignment output. takes two strings, a and b, and returns 0 if they are exactly alike, 1 if b is "greater" (i.e. should come after a in order), or -1 if a is "greater". the usage char signifies whether the rules for sorting words ('w') or files (any other char) should apply */
int mystrcmp(const char * a, const char * b, char usage) {
  printf("The strings to be compared are %s and %s\n", a, b);
  int len_a = strlen(a);
  int len_b = strlen(b);
  printf("The length of a is %d and the length of b is %d\n", len_a, len_b);
  int i = 0, j = 0;
  while (i < len_a && j < len_b) {

    if (a[i] == b[j]) {
      i++;
      j++;
    }
    else {
      break;
    }
  }
  if (i == len_a && j == len_b && len_a == len_b) {
    // both strings are the same
    printf("1\n");
    return 0;
  }
  else if ((isalpha(a[i]) && isalpha(b[j])) || (isdigit(a[i]) && isdigit(b[j])))  {
    printf("2\n");
    if (usage == 'w') {
      return a[i] < b[j] ? -1: 1;
    }
    return a[i] > b[j] ? -1: 1;
  } 
  else if (isdigit(a[i]) && isalpha(b[j])) {
    printf("3\n");
    if (usage == 'w') {
      return 1;
    }
    return -1;
  }
  else if (isalpha(a[i]) && isdigit(b[j])) {
    printf("4\n");
    if (usage == 'w') {
      return -1;
    }
    return 1;
  }
  else if (a[i] == '.') {
    printf("5\n");
    return -1;
  }
  else if (b[j] == '.') {
    printf("6\n");
    return 1;
  }
  printf("7\n");
  return 1;
}

/* function to insert a new node into the 2D list at the correct spot; CAUTION: the input parameter must be a proper string (i.e. ending in '\0')! */
int insert(char* s, char* f) {
  printf("Attempting to insert %s from file %s\n", s, f);
  /* if the head is NULL, then the linked list is empty so this inserts a node at the head */
  if (big_head == NULL) {
    BigNode* new_big = (BigNode*) malloc(sizeof(BigNode));
    new_big->word = s;
    new_big->little_size = 1;
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
    if (mystrcmp(s, tmp1->word, 'w') == 0) {
        /* the word does exist already; see whether or not its file exists yet */
//        printf("the word \"%s\" is already in the list. checking if the file exists too...\n", s);
        LittleNode* tmp = tmp1->little_head; //searching pointer
        while (tmp->next != NULL) {
          if (mystrcmp(f, tmp->file_name, 'f') == 0) {
            /* file does exist, so increment file's word count */
            printf("the word \"%s\" is already in file %s. increasing file's word count...\n", tmp1->word, tmp->file_name);
            printf("current count for %s is %d\n", tmp1->word, tmp->count);
            tmp->count++;
            printf("incremented. new count for %s is %d\n", tmp1->word, tmp->count);
            return 0;
          }

          tmp = tmp->next;
        }

        if (mystrcmp(f, tmp->file_name, 'f') == 0) {
          /* file does exist, so increment file's word count; this is outside the loop because we want to maintain the pointer to the last file node in the list, so we cut the while loop off before the end */
          printf("the word \"%s\" is already in file %s. increasing file's word count...\n", tmp1->word, tmp->file_name);
          printf("current count for %s is %d\n", tmp1->word, tmp->count);
          tmp->count++;
          printf("incremented. new count for %s is %d\n", tmp1->word, tmp->count);
          return 0;
        }

        /* file does not exist, so make a new file node for it */
        printf("the word \"%s\" is not in a file yet, so making a new file for it called %s\n", tmp1->word, f);
        LittleNode* new_little = (LittleNode*) malloc(sizeof(LittleNode));
        tmp1->little_size++;
        new_little->count++;
        new_little->file_name = f;
        new_little->next = NULL;
        tmp->next = new_little;
        return 0;
    }

    tmp1 = tmp1->next;
  }

  /* the word does not exist yet, so makes a new node for it */

  BigNode* new_big = (BigNode*) malloc(sizeof(BigNode));
  new_big->word = s;
  new_big->little_size = 1;
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
    if (mystrcmp(new_big->word, tmp1->word, 'w') <= 0) {
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

/* function to allocate a string composed of the digits of the given int and return a pointer to that string */
char* int2str(int x) {
  int digits = 1;
  int temp = x;
  while (x > 9) {
      x /= 10;
      digits++;
  }
  char* str = (char*)malloc(digits * sizeof(char) + 1);
  snprintf(str, 10, "%d", temp);

  return str;
}

/* diagnostic function to write the list to standard output with each string starting a new line */
int printList() {
  /* pointer to beginning of the list */
  BigNode* tmp1 = big_head;

  /* loop to iterate through the big list */
  while (tmp1 != NULL) {
    printf("%s\n", tmp1->word);

    LittleNode* tmp2 = tmp1->little_head;
    /* loop to iterate through the little list */
    while (tmp2 != NULL) {
      printf("  %s: %d\n", tmp2->file_name, tmp2->count);
      tmp2 = tmp2->next;
    }

    tmp1 = tmp1->next;
  }
  return 0;
}

/* function to output the conents of the list into an XML-formatted document named as the input */
int outputList(char* filename) {

  int outputFile = open(filename, O_RDWR|O_CREAT, 777);
//  int outputFile = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  

  write(outputFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<fileIndex>\n", 51);

  /* pointer to beginning of the list */
  BigNode* tmp1 = big_head;

  /* loop to iterate through the big list */
  while (tmp1 != NULL) {
    write(outputFile, "\t<word text=\"", 13);
//    printf("%s\n", tmp1->word);
    write(outputFile, tmp1->word, len(tmp1->word));
    write(outputFile, "\">\n", 3);

    LittleNode* tmp2 = tmp1->little_head;
    /* loop to iterate through the little list */
    while (tmp2 != NULL) {
      write(outputFile, "\t\t<file name=\"", 14);
      write(outputFile, tmp2->file_name, len(tmp2->file_name));
      write(outputFile, "\">", 2);

      char* counteger = int2str(tmp2->count);
      printf("%s\n", counteger);
      write(outputFile, counteger, len(counteger));
      free(counteger);

      write(outputFile, "</file>\n", 8);
      tmp2 = tmp2->next;
    }

    write(outputFile, "\t</word>\n", 9);
    tmp1 = tmp1->next;
  }

  write(outputFile, "</fileIndex>", 12);
  return 0;
}

/* function to compare the count of two file names to see which one comes first */
int count_sort(const void * x, const void * y) {
  LittleNode * xi = (LittleNode *) x; //the first node to compare
  LittleNode * yi = (LittleNode *) y; // the second node to compare
  if (xi->count == yi->count) {
    return mystrcmp(xi->file_name, yi->file_name, 'f');
  }
  return (int) (xi->count - yi->count);
}

/* main function used to sort the littlenodes in the list using the qsort library function */
void sort_littlenodes() {
  //printf("Inside sorting method\n");
  BigNode * tmp1 = big_head;
  /* loop to go through all the words in the big list */
  while (tmp1 != NULL) {
    LittleNode little_list [tmp1->little_size];
    LittleNode * tmp2 = tmp1->little_head;
    int i;
    int num_files = tmp1->little_size;
   /* loop to initialize an array of structs with the same information as the current keyword's list */
    for (i = 0; i < num_files; i++) {
      little_list[i].file_name = tmp2->file_name;
      little_list[i].count = tmp2->count;
      tmp2 = tmp2->next;
    }
    qsort(little_list, (size_t) num_files, sizeof(LittleNode), count_sort); //library qsort function that returns array sorted
    tmp2 = tmp1->little_head;
    /* loop to traverse all elements in the array and reset it to the new sorted elements. qsort returned in ascending order and we need it in descending order */
    for (i = num_files - 1; i >= 0; i--) {
      tmp2->file_name = little_list[i].file_name;
      tmp2->count = little_list[i].count;
      tmp2 = tmp2->next;
    }
    tmp1 = tmp1->next;
  }  
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
//      free(tmp3->file_name); //TODO make sure this is taken care of in other functions
      free(tmp3);
      tmp3 = tmp2;
    }

    free(tmp1->word);
    free(tmp1);
    tmp1 = tmp0;
  }
  return 0;
}

/* function to find if the input string has a forbidden name for a directory (i.e. do not try to open this) and if so, returns 0 */
int isValid(char* str) {
  if (mystrcmp(str, ".", 'w') == 0 || mystrcmp(str, "..", 'w') == 0 || mystrcmp(str, ".git", 'w') == 0 || mystrcmp(str, ".DS_store", 'w') == 0) {
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
  int i;
  for  (i = 0; i < len(file_name); i++) {
    if (isAlphanumeric(file_name[i])) {
      file_name[i] = tolower(file_name[i]);
    }
  }

  printf("File: %s passed in with descriptor %d\n", file_name, fd);
  char * buffer = (char *) calloc(101, sizeof(char)); //buffer that will hold 100 characters at a time. allocated 101 to allow 1 extra character for the null terminator
  printf("Empty buffer: %s\n", buffer);
  int buf_length = 100; //length of the string from the buffer
  i = 0; //this will be used to traverse the string from the buffer
  int j = 0; //this will be used to traverse the string for each token
  printf("The name of the file is: %s\n", file_name);
  int start = 0; //integer to tell whether we are starting a new token. 0 if false, 1 if true.
  int tokenized = 0; //integer to tell whether a string has been successfully tokenized. 0 if false, 1 if true.
  int bytesread = 0; //number of bytes that read has read (corner case for the end of the file)

  /* loop to traverse through the entire file */
  while ((bytesread = read(fd, buffer, 100)) != 0) {
    //  scan through the big string and construct the linked list
    for (i = 0; i < buf_length && i < bytesread; i++) {
      //printf("Current value of i is %d and character at this index is %c\n", i, buffer[i]);
      char * token = (char*) malloc ((buf_length - i + 1) * sizeof(char)); // this weird allocation was made to account for the biggest possible length of the substring at this point in the program, since we don't know how long each substring is beforehand
      tokenized = 0; //reset tokenized to 0 because we are creating the next token
      j = 0; //reset j to 0 so that the new token starts at index 0 rather than i
      start = 1; //we are starting a new token so set start to 1
      
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
      if (i == bytesread && (isAlphanumeric(buffer[bytesread - 1]) == 1 || isAlphanumeric(buffer[bytesread - 1]) == 2)) {
        char * partial_str = malloc (200*sizeof(char));
        int k = 0;
        token[j] = '\0';
        printf("Token to be partial: %s\n", token);
        
        for (k = 0; k < strlen(token); k++) {
          partial_str[k] = token[k];
        }
	bytesread = read(fd, buffer, 100);
        i = 0;
        printf("The string to be tokenized is %s\n", buffer);
        while ((isAlphanumeric(buffer[i]) == 1 || isAlphanumeric(buffer[i] == 2)) && i < buf_length) {
          i++;
        }
        printf("The new starting index of the buffer is %d\n", i);
        int l;
	for (l = 0; l < i; l++) {
          printf("%c\n", buffer[l]);
          partial_str[k] = buffer[l];
          k++;
        }
        partial_str[k] = '\0';
        printf("partial %s\n", partial_str);
        insert(partial_str, file_name);
        //free(partial_str);
        continue;
      }
      /* if enough memory was alloted for the string then we can insert it into the linked list */
      else if (tokenized == 1) {
        token[j] = '\0';
        insert(token, file_name);
        //printf("insert complete. printing current list...\n");
      }

      
    }
  }
  printf("List printed inside of processFile:\n");
  printList();
  free(buffer);
//  free(partial_str);
  close(fd);
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
  printf("\n\n\n");

  /* checks that the input has two and only two arguments, else exit with error */
  if (argc != 3) {
    printf("Error! Program must be run with exactly two arguments, but %d were found. Exiting...\n", argc - 1);
    exit(0);
  }

  /* checks for a preexisting file of the specified name and gives the option to overwrite or leave it if so */
  if (access(argv[1], F_OK) != -1 ) {
    char ovr[256];
    ovr[0] = '\n';
 
    printf("A file named %s already exists. Enter \"c\" to continue or \"q\" to quit.\n", argv[1]);
    scanf("%s", ovr);
    
    if (ovr[0] == 'q') {
      printf("Not overwriting preexisting file.\n");
      exit(0);
    }

    int rem;
    rem = remove(argv[1]);

    if (rem) {
      printf("Error! Attempted to overwrite file %s but it could not be altered. Exiting...\n", argv[1]);
      exit(0);
    }
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

  int file_made = 0;

  // checks to see if input is a directory
  if (fd == -2) {
    // input is a directory, so treat it as one
      printf("Reading %s as a directory...\n", argv[2]);
      processDir(argv[2]);
  } else if (fd >= 0) {
    // input is not a directory, so treat it as a single file

      char * file_name = (char *) malloc (sizeof(char) * 263); //largest possible length of a file's name
      file_made = 1;
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

//  sortCounts();
    sort_littlenodes();

  outputList(argv[1]);

  if (file_made) {
//    free(file_name); TODO figure out how to free this somewhere
  }

  freeList();
  return 0;
}


/* things TODO:

[X] make file reader that creates a buffer and tokenizes each word, converting each character to lowercase

[ ] make sure that each input to insert() is a proper string! (ends in '\0')

[ ] sort each little list by its occurrence number at the end of the program

[X] in processDir, ignore .git and .DS_store and . and ..

[ ] free everything that was malloc'd in processFile()

[ ] process list into XML format

[ ] add to big list in correct order (not from strcmp)


   types of input to account for:

[X] file only

[X] directory only

[X] directory leading to file

[X] directory leading to another directory

[X] directory leading to another directory leading to a file

[X] invalid input types

[ ] a file or directory that the user does not have permission to access (?)

[ ] names for output files that will result in invalid filenames

*/
