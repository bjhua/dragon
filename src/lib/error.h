#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

#define Error_bug(s)                                            \
  do{                                                           \
    fprintf (stderr,                                            \
             "Compiler bug: %s, %d, %s",                        \
             __FILE__,                                          \
             __LINE__,                                          \
             s);                                                \
    fflush (stderr);                                            \
    exit (1);                                                   \
  }while (0)

#define Error_bug2(s, i)                                        \
  do{                                                           \
    fprintf (stderr,                                            \
             "Compiler bug: %s, %d, %s, %d",                    \
             __FILE__,                                          \
             __LINE__,                                          \
             s,                                                 \
             i);                                                \
    fflush (stderr);                                            \
    exit (1);                                                   \
  }while (0)

#define Error_impossible()                                      \
  do{                                                           \
    fprintf (stderr,                                            \
             "Compiler bug: %s, %d, impossible",                \
             __FILE__,                                          \
             __LINE__);                                         \
    fflush (stderr);                                            \
    *((int *)1) = 0;                                            \
  }while (0)

void Error_error (const char *s);
void Error_error2 (const char *s1, const char *s2);

#define Error_bomb() *(int*)1=0

#endif
