/*
 * parser.c
 * Where the actual parsing happens.
 * Creates a linked list of either words or numbers (depending on the
 * problem) from a file.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>

#include "parsers.h"


/*
 * Parses an input file, and constructs a WCLinkedList struct of the words
 * in the file.
 *
 * The basic idea is that we read into a buffer one byte at a time. If we
 * hit a delimeter, then we copy the buffer into a token, and clear the buffer.
 *
 * If we don't hit a delimeter before we reach the end of the buffer, then
 * we copy the entire buffer somewhere else, clear the buffer, and start reading
 * from the beginning again. When we eventually do reach a delimeter, we put the
 * copied string from before together with the current buffer to make one big token.
 */

LinkedList *word_count_parse(char *file)
{
    int fd = open(file, O_RDONLY);         // Only have to read from the file.

    if (fd < 0) {
        // Error opening file.
        fprintf(stderr, "Error in %s at line %d: open(): %s\n",
                __FILE__, __LINE__, strerror(errno));
    }

    LinkedList * word_list = create_empty_list();
    Node * word_ptr = word_list->head;

    char buffer[WC_BUF_SIZE];     // Buffer we're reading into.
    memset(buffer, 0, WC_BUF_SIZE);     // Zero out the buffer.

    char * token = NULL;
    char * partial_token= NULL;    // Stores extra string if buffer overflows.

    int read_bytes;
    int i = 0;

    while (i < WC_BUF_SIZE - 1) {
        // Read into buffer, one character at a time.
        read_bytes = read(fd, buffer + i, 1);

        if (read_bytes == 0) {
            // At the end of the file, so we have to make a token out of
            // whatever we have in token and partial_token.

            // Allocate what we scanned, plus 1 byte for null terminator.
            token = (char *) calloc(sizeof(char) * i + 1, sizeof(char) * i + 1);
            strncpy(token, buffer, i);

            // Merge overflow (if we have any) and the token we just scanned.
            token = merge_tokens(partial_token, token);

            if (strcmp(token, "") != 0) {
                // Don't have a blank token, so insert into the list.
                if (word_list->head != NULL) {
                  word_ptr->next = create_node(token, 1);
                  word_ptr = word_ptr->next;
                  word_list->size++;
                }
                else {
                  word_list->head = create_node(token, 1);
                  word_ptr = word_list->head;
                  word_list->size++;
                }
            }
            break;              // Done reading from file.
        }

        buffer[i] = tolower(buffer[i]);     // Convert all characters to lowercase.

        if (!isalnum(buffer[i])) {
            // Delimeters are *pretty much* all nonalphanumeric characters,
            // except for a couple, and it's reasonable to assume we won't have
            // tabs or anything like that.

            // Allocate what we scanned, plus 1 byte for null terminator.
            token = (char *) calloc(sizeof(char) * i + 1, sizeof(char) * i + 1);
            strncpy(token, buffer, i);

            // Merge the overflow (if we have any) and the token we just scanned.
            token = merge_tokens(partial_token, token);
            if (strcmp(token, "") != 0) {
                if (word_list->head != NULL) {
                    word_ptr->next = create_node(token, 1);
                    word_ptr = word_ptr->next;
                    word_list->size++;
                }
                
                else {
                    word_list->head = create_node(token, 1);
                    word_ptr = word_list->head;
                    word_list->size++;
                }

                // Reset all variables, and start the token search again.
                i = 0;
                memset(buffer, 0, WC_BUF_SIZE);
                token = NULL;
                partial_token = NULL;

            }
            
        } else {
            i++;
            if (i - 1 >= WC_BUF_SIZE + 1) {
                // We're at the end of the buffer and still haven't found a
                // delimeter, so append the current buffer to the end of
                // the partial_token.
                char *tmp = malloc(sizeof(char) * WC_BUF_SIZE);

                // Zero this out so we have a guaranteed '\0' at the end.
                memset(tmp, 0, sizeof(char) * WC_BUF_SIZE);
                strncpy(tmp, buffer, sizeof(char) * WC_BUF_SIZE);

                // Append to the end of the partial token.
                partial_token = merge_tokens(partial_token, tmp);

                // Reset variables and start scanning for tokens again.
                i = 0;
                memset(buffer, 0, WC_BUF_SIZE);
            }
        }
    }

    return word_list;
}

/*
 * Concatenates the partial and buffer strings into one string.
 * If partial is NULL, it just returns buffer.
 * Note: both strings passed in should be malloc'ed, so they will be freed
 * in this function.
 */
char *merge_tokens(char *partial, char *buffer)
{
    int partial_size;
    int buffer_size;

    if (partial) {
        partial_size = strlen(partial);
    } else {
        partial_size = 0;
    }

    buffer_size = strlen(buffer);

    char *new_partial = malloc(sizeof(char) * (partial_size + buffer_size) + 1);
    strncpy(new_partial, partial, partial_size);
    strcat(new_partial, buffer);
    new_partial[partial_size + buffer_size] = '\0';

    free(partial);

    return new_partial;
}
