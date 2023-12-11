#include "dlist.h"
#include "mem.h"
#include <assert.h>

#define T Dlist_t

T Dlist_new() {
    T t;

    Mem_NEW(t);
    t->data = 0;
    t->next = t;
    t->prev = t;
    return t;
}

void Dlist_insertLast(T t, Poly_t data) {
    assert(t);
    assert(data);

    T tmp;
    Mem_NEW(tmp);
    tmp->data = data;
    assert(t);
    tmp->prev = t->prev;
    tmp->prev->next = tmp;
    tmp->next = t;
    t->prev = tmp;
    //    return;
}

//static int Dlist_length(T t) {
//    T p = t->next;
//    int i = 0;
//
//    while (p != t) {
//        ++i;
//        p = p->next;
//    }
//    return i;
//}

#undef T
