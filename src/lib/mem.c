#include <assert.h>
#include <stdio.h>

#include "gc.h"
#include "mem.h"

long Mem_allocated = 0;
long Mem_initFlag = 0;

/* the accumulated time should include GC time */
static clock_t totalClocks = 0;

void Mem_init(void) {
    //GC_INIT();
    totalClocks = 0;
}

#undef GC_MALLOC
#define GC_MALLOC malloc

void *Mem_alloc(long size) {
    void *p;
    clock_t start, finish;

    start = clock();
    p = GC_MALLOC((unsigned long) size);
    if (0 == p)
        Error_error("allocation failed\n");

    // initialized it with zeros to catch bugs
    {
        char *cp = (char *) p;
        assert(cp);
        for (long i = 0; i < size; i++)
            cp[i] = '\0';
    }

    // status info
    Mem_allocated += size;
    finish = clock();
    totalClocks += (finish - start);
    return p;
}

clock_t Mem_getClock(void) {
    return totalClocks;
}

#define ONEM (1024 * 1024)

void Mem_status(void) {
    unsigned long total = 0, sinceLast = 0;

    //total = GC_get_total_bytes();
    //sinceLast = GC_get_bytes_since_gc();
    printf("Heap status:\n"
           "  Total allocation        : %lu bytes (~%ldM)\n"
           "  Allocation since last GC: %ld bytes (~%ldM)\n",
           total, total / ONEM,
           sinceLast, sinceLast / ONEM);

    Mem_allocated = 0;
    return;
}
