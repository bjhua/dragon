#include "atype.h"
#include "../lib/mem.h"
#include "../lib/unused.h"
#include <assert.h>

#define T Atype_t

T Atype_new_int(void) {
    T p;

    Mem_NEW(p);

    p->kind = ATYPE_INT;
    return p;
}

T Atype_new_int_array(void) {
    T p;

    Mem_NEW(p);
    p->kind = ATYPE_INT_ARRAY;
    return p;
}

T Atype_new_string(String_t x) {
    UNUSED(x);

    T p;

    Mem_NEW(p);
    p->kind = ATYPE_STRING;
    return p;
}

T Atype_new_string_array(void) {
    T p;

    Mem_NEW(p);
    p->kind = ATYPE_STRING_ARRAY;
    return p;
}

T Atype_new_class(Id_t id) {
    T p;

    Mem_NEW(p);

    p->kind = ATYPE_CLASS;
    p->u.id = id;
    return p;
}

T Atype_new_class_array(Id_t id) {
    T p;

    Mem_NEW(p);

    p->kind = ATYPE_CLASS_ARRAY;
    p->u.id = id;
    return p;
}

T Atype_new_fun(List_t from, T to) {
    T p;

    Mem_NEW(p);

    p->kind = ATYPE_FUN;
    p->u.fun.from = from;
    p->u.fun.to = to;
    return p;
}

int Atype_maybeGc(T ty) {
    assert(ty);
    switch (ty->kind) {
        case ATYPE_INT:
        case ATYPE_STRING:
            return 0;
        case ATYPE_INT_ARRAY:
        case ATYPE_STRING_ARRAY:
        case ATYPE_CLASS:
        case ATYPE_CLASS_ARRAY:
            return 1;
        case ATYPE_FUN:
            Error_impossible();
            return 0;
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
}

String_t Atype_toString(T ty) {
    assert(ty);
    switch (ty->kind) {
        case ATYPE_INT:
            return "int";
        case ATYPE_INT_ARRAY:
            return "int[]";
        case ATYPE_STRING:
            return "string";
        case ATYPE_STRING_ARRAY:
            return "string[]";
        case ATYPE_CLASS:
            return Id_toString(ty->u.id);
        case ATYPE_CLASS_ARRAY:
            return String_concat(Id_toString(ty->u.id), "[]", 0);
        case ATYPE_FUN:
            return "<fun>";
        default:
            Error_impossible();
            return "<junk>";
    }
    Error_impossible();
}

#undef T
