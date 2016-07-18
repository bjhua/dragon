#ifndef TRACE_H
#define TRACE_H

#include <time.h>
#include <stdio.h>
#include "list.h"

#include "string.h"

void Trace_indent ();
void Trace_unindent ();
void Trace_spaces ();
int Trace_lookup (char *);
void Trace_insert (char *);
List_t Trace_allFuncs ();
void Trace_reset ();
String_t Trace_junk ();
String_t Trace_junk2 (void *);

#ifndef NDEBUG

#define Trace_TRACE(s, f, x, dox, r, dor)               \
  do {                                                  \
    clock_t start, finish;                              \
    int exists = Trace_lookup (s);                      \
    if (exists) {                                       \
      Trace_spaces ();                                  \
      printf ("%s starting\n", s);                      \
      Trace_spaces ();                                  \
      printf ("arg is:");                               \
      dox x;                                            \
      printf ("\n");                                    \
      Trace_indent ();                                  \
      start = clock ();                                 \
    }                                                   \
    r = f x;                                            \
    if (exists) {                                       \
      Trace_unindent ();                                \
      Trace_spaces ();                                  \
      printf (s " finished\n");                         \
      Trace_spaces ();                                  \
      printf ("result is: ");                                   \
      dor (r);                                                  \
      Trace_spaces ();                                          \
      finish = clock ();                                        \
      printf (" @time: %lf\n",                                  \
              ((double)(finish-start))                          \
              /CLOCKS_PER_SEC);                                 \
    }                                                           \
  } while (0)

#else 

#define Trace_TRACE(s, f, x, toStringx, r, toStringr) \
  do {                                                \
    r = f x;                                          \
  } while (0)

#endif

#endif
