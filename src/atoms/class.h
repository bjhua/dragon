#ifndef CLASS_H
#define CLASS_H

#include "atype.h"
#include "id.h"
#include "../lib/file.h"
#include "../lib/string.h"
#include "../lib/list.h"

#define T Class_t

typedef struct T *T;

struct T {
    Id_t name;
    // List<Dec_t>
    List_t decs;
};

T Class_new(Id_t, List_t);

String_t Class_toString(T);

File_t Class_print(File_t file, T);

#undef T

#endif
