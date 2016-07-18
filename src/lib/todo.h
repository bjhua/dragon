#ifndef TODO_H
#define TODO_H

#include <stdio.h>
#include <stdlib.h>

#define TODO \
  do {       \
    printf("\n\
#############################################################\n\
@@ TODO: fill in your code.\n\
@@ in file: \"%s\", function: \"%s\"\n\
@@ at line: %d\n\
#############################################################\n",\
           __FILE__, __func__, __LINE__);                        \
    exit(1);                                                     \
  } while (0)

#endif
