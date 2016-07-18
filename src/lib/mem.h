#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <time.h>
/*#include "control.h"*/
#include "error.h"

extern int Mem_allocated;
extern int Mem_initFlag;

#define Mem_NEW(p)                              \
  do{                                           \
    (p) = Mem_alloc (sizeof (*(p)));            \
} while (0)

#define Mem_NEW_SIZE(p, n)                      \
  do {                                          \
    if (n<=0)                                   \
      Error_error ("invalid buffer size");      \
    (p) = Mem_alloc ((n)*sizeof (*(p)));        \
  } while (0)

void *Mem_alloc (unsigned int size);
clock_t Mem_getClock ();
void Mem_init ();
void Mem_status ();

#endif
