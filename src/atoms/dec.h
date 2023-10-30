#ifndef DEC_H
#define DEC_H

#include "../lib/file.h"
#include "atype.h"
#include "id.h"

#define T Dec_t

typedef struct T *T;

struct T {
    Atype_t ty;
    Id_t id;
};

T Dec_new(Atype_t, Id_t);

// The variable in this declaration may be GCed.
int Dec_maybeGc(T);

String_t Dec_toString(T);

File_t Dec_print(File_t file, T);

File_t Dec_printAsArg(File_t file, T);

File_t Dec_printAsLocal(File_t file, T);

#undef T

#endif
