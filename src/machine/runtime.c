#include "runtime.h"

Id_t Runtime_array = 0;;
Id_t Runtime_class = 0;

void Runtime_init() {
    Runtime_class =
            Id_fromString("Dragon_Runtime_alloc_class");

    Runtime_array =
            Id_fromString("Dragon_Runtime_alloc_array");
    return;
}

