#include "list-pair.h"
#include "tuple.h"
#include "unused.h"
#include <assert.h>

#define T ListPair_t
#define L List_t
#define P Poly_t

T ListPair_new(void *x, void *y) {
    UNUSED(x);
    UNUSED(y);
    return List_new();
}

void ListPair_insertLast(T l, P x, P y) {
    assert(l);
    List_insertLast(l, Tuple_new(x, y));
}

L ListPair_first(T l) {
    L tmp, p;

    assert(l);
    tmp = List_new();
    p = List_getFirst(l);
    while (p) {
        List_insertLast(tmp, Tuple_first(p->data));
        p = p->next;
    }
    return tmp;
}

L ListPair_second(T l) {
    L tmp, p;

    assert(l);
    tmp = List_new();
    p = List_getFirst(l);
    while (p) {
        List_insertLast(tmp, Tuple_second(p->data));
        p = p->next;
    }
    return tmp;
}

int ListPair_forall(T x, int (*pred)(Poly_t, Poly_t)) {
    T tmp;
    Tuple_t tuple;

    assert(x);
    assert(pred);
    tmp = List_getFirst(x);
    while (tmp) {
        tuple = (Tuple_t) tmp->data;
        if (!pred(Tuple_first(tuple), Tuple_second(tuple)))
            return 0;
        tmp = tmp->next;
    }
    return 1;
}

#undef L
#undef T
#undef P
