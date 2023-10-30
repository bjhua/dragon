#ifndef LIST_PAIR_H
#define LIST_PAIR_H

#include "list.h"
#include "poly.h"

#define T ListPair_t
#define L List_t
#define P Poly_t

typedef L T;

/* if x and y are of different size, return 0. */
T ListPair_new(void *x, void *y);

void ListPair_insertLast(T l, P x, P y);

L ListPair_first(T l);

L ListPair_second(T l);

int ListPair_forall(T x, int (*pred)(Poly_t, Poly_t));

#undef L
#undef T
#undef P

#endif
