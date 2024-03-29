#include "double.h"
#include "mem.h"
#include <stdio.h>

#define BUF_SIZE 1024

String_t Double_toString(double f) {
    char *temp;
    Mem_NEW_SIZE(temp, BUF_SIZE);
    /* Note that I initially want to write:
     *   snprintf (temp, BUF_SIZE, "%d", i);
     * but it seems that this version of GCC I'm using has a bug on
     * the function "snprintf". So I've to write this unsafe "sprintf".
     */
    snprintf(temp, BUF_SIZE - 1, "%lf", f);
    return temp;
}

#undef BUF_SIZE
