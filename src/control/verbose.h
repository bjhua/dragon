#ifndef VERBOSE_H
#define VERBOSE_H

#include "../lib/mem.h"
#include "../lib/trace.h"
#include <stdio.h>
#include <time.h>

#define Verbose_TRACE(s, f, x, r, level)                                  \
    do {                                                                  \
        clock_t start = clock(), finish = clock();                        \
        clock_t gcStart = clock(), gcFinish = clock();                    \
        int exists = Control_Verb_order(level, Control_verbose);          \
        if (exists) {                                                     \
            Trace_spaces();                                               \
            printf("%s starting\n", s);                                   \
            Trace_indent();                                               \
            start = clock();                                              \
            gcStart = Mem_getClock();                                     \
        }                                                                 \
        r = f x;                                                          \
        if (exists) {                                                     \
            Trace_unindent();                                             \
            Trace_spaces();                                               \
            printf("%s finished", s);                                     \
            finish = clock();                                             \
            gcFinish = Mem_getClock();                                    \
            if (Control_Verb_order(VERBOSE_DETAIL, Control_verbose)) {    \
                printf("  @time: %.3lf (GC: %.3lf)",                      \
                       ((double) (finish - start)) / CLOCKS_PER_SEC,      \
                       ((double) (gcFinish - gcStart)) / CLOCKS_PER_SEC); \
            }                                                             \
            printf("\n");                                                 \
        }                                                                 \
    } while (0)

#endif
