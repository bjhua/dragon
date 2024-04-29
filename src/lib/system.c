#include "system.h"
#include "error.h"
#include <stdlib.h>

void System_run(char *s) {
    if (!s)
        Error_error("invalid command\n");

    // the return value is platform-dependent, so it's
    // useless to check this.
    system(s);
}
