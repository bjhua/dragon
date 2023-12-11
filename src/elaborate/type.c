#include "type.h"
#include "../lib/error.h"
#include "../lib/list-pair.h"
#include "../lib/mem.h"
#include <assert.h>
#include <stdarg.h>

#define T Type_t

T Type_new_int() {
    T t;

    Mem_NEW(t);
    t->kind = TYPE_A_INT;
    t->isArray = 0;
    return t;
}

T Type_new_string() {
    T t;

    Mem_NEW(t);
    t->kind = TYPE_A_STRING;
    t->isArray = 0;
    return t;
}

T Type_new_ns() {
    T t;

    Mem_NEW(t);
    t->kind = TYPE_A_NS;
    t->isArray = 0;
    return t;
}

T Type_new_class(AstId_t name) {
    T t;

    Mem_NEW(t);
    t->kind = TYPE_A_CLASS;
    t->isArray = 0;
    t->u.className = name;
    return t;
}

T Type_new_array(T t) {
    T p;

    assert(t);

    Mem_NEW(p);
    p->kind = t->kind;
    p->isArray = 1;
    p->u = t->u;
    return p;
}


void Type_set_array(T t) {
    assert(t);

    if (t->isArray)
        Error_impossible();

    t->isArray = 1;
    return;
}

T Type_clearArray(T t) {
    assert(t);

    T p;
    Mem_NEW(p);

    *p = *t;
    if (!p->isArray)
        Error_impossible();

    p->isArray = 0;
    return p;
}

T Type_new_product(T x, ...) {
    List_t list = List_new();
    va_list ap;
    T current, t;

    List_insertLast(list, x);
    va_start(ap, x);
    current = va_arg(ap, T);
    while (current) {
        List_insertLast(list, current);
        current = va_arg(ap, T);
    }
    va_end(ap);
    Mem_NEW(t);
    t->kind = TYPE_A_PRODUCT;
    t->isArray = 0;
    t->u.product = list;
    return t;
}

T Type_new_product2(List_t list) {
    T t;
    Mem_NEW(t);
    t->kind = TYPE_A_PRODUCT;
    t->isArray = 0;
    t->u.product = list;
    return t;
}

T Type_new_fun(T from, T to) {
    T t;
    Mem_NEW(t);
    t->kind = TYPE_A_FUN;
    t->isArray = 0;
    t->u.fun.from = from;
    t->u.fun.to = to;
    return t;
}

int Type_equals(T t1, T t2) {
    assert(t1);
    assert(t2);

    if (t1->isArray) {
        if (t2->isArray)
            ;
        else if (t2->kind == TYPE_A_NS)
            return 1;
    } else {
        if (t2->isArray)
            return 0;
        else
            ;
    }

    switch (t1->kind) {
        case TYPE_A_INT:
            switch (t2->kind) {
                case TYPE_A_INT:
                    return 1;
                default:
                    return 0;
            }
        case TYPE_A_STRING:
            switch (t2->kind) {
                case TYPE_A_STRING:
                    return 1;
                default:
                    return 0;
            }
        case TYPE_A_NS:
            Error_bug("impossible");
            return 0;
        case TYPE_A_CLASS:
            switch (t2->kind) {
                case TYPE_A_NS:
                    return 1;
                case TYPE_A_CLASS:
                    if (AstId_equals(t1->u.className,
                                     t2->u.className))
                        return 1;
                    return 0;
                default:
                    return 0;
            }
        case TYPE_A_PRODUCT:
            switch (t2->kind) {
                case TYPE_A_PRODUCT: {
                    ListPair_t pairs = ListPair_new(t1->u.product,
                                                    t2->u.product);
                    if (!pairs)
                        return 0;
                    return ListPair_forall(pairs,
                                           (int (*)(Poly_t, Poly_t)) Type_equals);
                }
                default:
                    return 0;
            }
        case TYPE_A_FUN:
            Error_impossible();
            return 0;
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

int Type_equals_int(T t) {
    return Type_equals(Type_new_int(), t);
}

int Type_equals_string(T t) {
    return Type_equals(Type_new_string(), t);
}

//static int Type_equals_ns(T t) {
//    return Type_equals(Type_new_ns(), t);
//}

String_t Type_toString(T t) {
    String_t array = (t->isArray) ? "[]" : "";

    assert(t);
    switch (t->kind) {
        case TYPE_A_INT:
            return String_concat("int",
                                 array,
                                 0);
        case TYPE_A_STRING:
            return String_concat("string",
                                 array,
                                 0);
        case TYPE_A_NS:
            return String_concat("null",
                                 array,
                                 0);
        case TYPE_A_CLASS:
            return String_concat("class",
                                 AstId_toString(t->u.className),
                                 array,
                                 0);
        case TYPE_A_PRODUCT: {
            if (t->isArray)
                Error_impossible();

            return List_toString(t->u.product,
                                 ", ",
                                 (Poly_tyToString) Type_toString);
        }
        case TYPE_A_FUN: {
            if (t->isArray)
                Error_impossible();

            return (String_concat(Type_toString(t->u.fun.from),
                                  " -> ",
                                  Type_toString(t->u.fun.to),
                                  0));
        }
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

Tuple_t Type_dest_fun(T t) {
    assert(t);
    switch (t->kind) {
        case TYPE_A_FUN:
            return Tuple_new(t->u.fun.from, t->u.fun.to);
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

#undef T
