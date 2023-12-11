#ifndef TRACE_H
#define TRACE_H

#include "list.h"
#include <stdio.h>
#include <time.h>

#include "string.h"

void Trace_indent(void);

void Trace_unindent(void);

void Trace_spaces(void);

int Trace_lookup(char *);

void Trace_insert(char *);

List_t Trace_allFuncs(void);

void Trace_reset(void);

String_t Trace_junk(void);

String_t Trace_junk2(void *);

#ifndef NDEBUG

#define Trace_TRACE(s, f, x, dox, r, dor)                         \
    do {                                                          \
        clock_t start = clock(), finish = clock();                \
        int exists = Trace_lookup(s);                             \
        if (exists) {                                             \
            Trace_spaces();                                       \
            printf("%s starting\n", s);                           \
            Trace_spaces();                                       \
            printf("arg is:");                                    \
            dox x;                                                \
            printf("\n");                                         \
            Trace_indent();                                       \
            start = clock();                                      \
        }                                                         \
        r = f x;                                                  \
        if (exists) {                                             \
            Trace_unindent();                                     \
            Trace_spaces();                                       \
            printf(s " finished\n");                              \
            Trace_spaces();                                       \
            printf("result is: ");                                \
            dor(r);                                               \
            Trace_spaces();                                       \
            finish = clock();                                     \
            printf(" @time: %lf\n",                               \
                   ((double) (finish - start)) / CLOCKS_PER_SEC); \
        }                                                         \
    } while (0)

#else

#define Trace_TRACE(s, f, x, toStringx, r, toStringr) \
    do {                                              \
        r = f x;                                      \
    } while (0)

#endif

#endif
