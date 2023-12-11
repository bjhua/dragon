#include "region.h"
#include "../lib/mem.h"
#include <assert.h>

#define T Region_t

struct T {
    Coordinate_t from;
    Coordinate_t to;
};

T Region_new(Coordinate_t from, Coordinate_t to) {
    T t;
    Mem_NEW(t);
    t->from = from;
    t->to = to;
    return t;
}

T Region_bogus() {
    Coordinate_t bogus = Coordinate_bogus();
    return Region_new(bogus, bogus);
}

Coordinate_t Region_from(T t) {
    assert(t);
    return t->from;
}

Coordinate_t Region_to(T t) {
    assert(t);
    return t->to;
}

String_t Region_toString(T t) {
    assert(t);
    return String_concat("from ", Coordinate_toString(t->from),
                         ", to ", Coordinate_toString(t->to),
                         0);
}

#undef T
