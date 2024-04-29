#ifndef ATYPE_H
#define ATYPE_H

#include "../lib/string.h"
#include "id.h"

#define T Atype_t

typedef struct T *T;

struct T {
    enum {
        ATYPE_INT,
        ATYPE_STRING,
        ATYPE_CLASS,
        ATYPE_INT_ARRAY,
        ATYPE_STRING_ARRAY,
        ATYPE_CLASS_ARRAY,
        ATYPE_FUN
    } kind;
    union {
        Id_t id;
        struct {
            // List<T>
            List_t from;
            T to;
        } fun;
    } u;
};

T Atype_new_int(void);
T Atype_new_int_array(void);
T Atype_new_string(const String_t);
T Atype_new_string_array(void);
T Atype_new_class(Id_t);
T Atype_new_class_array(Id_t);
T Atype_new_fun(List_t from, T to);
int Atype_maybeGc(T);
String_t Atype_toString(T);

#undef T

#endif
