#include "dec.h"
#include "../lib/mem.h"
#include <assert.h>

#define T Dec_t

T Dec_new(Atype_t ty, Id_t id) {
    T p;

    Mem_NEW(p);
    p->ty = ty;
    p->id = id;
    return p;
}

int Dec_maybeGc(T dec) {
    return Atype_maybeGc(dec->ty);
}

String_t Dec_toString(T dec) {
    assert(dec);
    return String_concat(Atype_toString(dec->ty), " ", Id_toString(dec->id), 0);
}

File_t Dec_print(File_t file, T dec) {
    assert(file);
    assert(dec);

    fprintf(file, "%s %s", Atype_toString(dec->ty), Id_toString(dec->id));
    return file;
}

File_t Dec_printAsArg(File_t file, T dec) {
    assert(file);
    assert(dec);

    fprintf(file, "%s %s, ", Atype_toString(dec->ty), Id_toString(dec->id));
    return file;
}

File_t Dec_printAsLocal(File_t file, T dec) {
    assert(file);
    assert(dec);

    fprintf(file, "\t%s %s;\n", Atype_toString(dec->ty), Id_toString(dec->id));
    return file;
}


#undef T
