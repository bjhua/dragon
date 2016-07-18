#include "include/gc.h"
#include <assert.h>
#include <stdio.h>

int notmain()
{
  int i;

  GC_INIT();	/* Optional on Linux/X86; see below.  */
  for (i = 0; i < 10000000; ++i)
   {
     int *q = (int *) GC_MALLOC_ATOMIC(sizeof(int));
     if (i % 100000 == 0)
       printf("Heap size = %d\n", GC_get_heap_size());
   }
  return 0;
}
