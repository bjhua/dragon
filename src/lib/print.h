#ifndef PRINT_H
#define RPINT_H

#include <stdio.h>
extern int result;
void printIndent ();
void indent ();
void unindent ();

#define Trace_trace(s, f, x, r)                      \
  do {                                               \
    printIndent();                                   \
    printf("%s starts\n", s);                        \
    indent();                                        \
    r = f x;                                         \
    unindent();                                      \
    printIndent();                                   \
    printf("%s ends\n", s);                          \
  }while (0)

void Dp_print (char *s);

#endif
