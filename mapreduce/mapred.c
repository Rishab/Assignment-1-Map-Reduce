/*
 * mapred.c
 * Testing program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsers.h"

int main(int argc, char **argv)
{
    WCLinkedList *list = word_count_parse("./test/file1.txt");
    print_wc_list(list);
    return 0;
}