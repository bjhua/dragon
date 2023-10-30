#include <stdio.h>
#include "string.h"
#include "mem.h"
#include "int.h"

const int Int_zero = 0;

#define BUF_SIZE 1024

String_t Int_toString(long i) {
    char *temp;
    Mem_NEW_SIZE(temp, BUF_SIZE);
    /* Note that I initially want to write:
     *   snprintf (temp, BUF_SIZE, "%d", i);
     * but it seems that this version of GCC I'm using
     * has a bug on the function "snprintf". So I've
     * to write this unsafe "sprintf".
     */
    sprintf (temp, "%ld", i);
    return temp;
}

// The stdlib has "strtol" family of conversion functions,
// but that is not what I want. Here, I want the code 
// issue error for illegal strings, such as these:
//   "123xy"
// For now, it only works with deciminal numbers, but
// this algorithm is of no difficulty to scale to 
// other numbers.
// Return 0 for failure, 1 for success.
long Int_fromString(String_t s, long *result) {
    unsigned char c = 0;
    long sum = 0;

    while ((c = *s++)) {
        if (c >= '0' && c <= '9')
            sum = sum * 10 + (c - '0');
        else return 0;
    }
    *result = sum;
    return 1;
}


#undef BUF_SIZE

