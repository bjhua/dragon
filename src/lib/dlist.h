#ifndef DLIST_H
#define DLIST_H

// Doubly-linked list.

#include "poly.h"

#define T Dlist_t

typedef struct T *T;

// CDT
struct T {
    Poly_t data;
    T next;
    T prev;
};


T Dlist_new(void);

void Dlist_insertLast(T, Poly_t);


#undef T

#endif
