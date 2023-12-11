#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <time.h>
/*#include "control.h"*/
#include "error.h"

extern long Mem_allocated;
extern long Mem_initFlag;

#define Mem_NEW(p)                     \
    do {                               \
        (p) = Mem_alloc(sizeof(*(p))); \
    } while (0)

#define Mem_NEW_SIZE(p, n)                                          \
    do {                                                            \
        if (n <= 0)                                                 \
            Error_error("invalid buffer size");                     \
        unsigned long new_n = ((unsigned long) (n)) * sizeof(*(p)); \
        (p) = Mem_alloc((long) new_n);                              \
    } while (0)

void *Mem_alloc(long size);

clock_t Mem_getClock(void);

void Mem_init(void);

void Mem_status(void);

#endif
