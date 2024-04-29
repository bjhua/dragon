#include "coordinate.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include <assert.h>

#define T Coordinate_t

struct T {
    String_t file;
    int line;
    int column;
};

T Coordinate_new(String_t file, int line, int column) {
    T t;
    Mem_NEW(t);
    t->file = file;
    t->line = line;
    t->column = column;
    return t;
}

T Coordinate_bogus(void) {
    return Coordinate_new("<bogus>",
                          -1,
                          -1);
}

String_t Coordinate_file(T t) {
    assert(t);
    return t->file;
}

int Coordinate_column(T t) {
    assert(t);
    return t->column;
}

int Coordinate_line(T t) {
    assert(t);
    return t->line;
}

String_t Coordinate_toString(T t) {
    assert(t);
    return String_concat("(file \"", t->file,
                         "\", line ", Int_toString(t->line),
                         ", column ", Int_toString(t->column),
                         ")",
                         0);
}

#undef T
