#include "io.h"
#include <assert.h>
#include <stdio.h>

int Io_printSpaces(long n) {
    assert(n >= 0);
    while (n-- > 0) {
        printf(" ");
    }
    return 0;
}
