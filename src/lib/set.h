#ifndef SET_H
#define SET_H

#include "poly.h"
#include "list.h"

#define T Set_t

typedef struct T *T;

T Set_new(Poly_tyEquals equals);

void Set_delete(T set, Poly_t x);

void Set_deleteAll(T set1, Poly_tyPred pred);

int Set_exists(T set, Poly_t x);

T Set_fromList(Poly_tyEquals equals, List_t list);

void Set_foreach(T set, Poly_tyVoid f);

void Set_insert(T set, Poly_t x);

T Set_intersection(T, T);

int Set_isEmpty(T);

// Remove one element from a set.
Poly_t Set_removeOne(T);

T Set_singleton(Poly_tyEquals equals, Poly_t x);

List_t Set_toList(T set);

T Set_union(T, T);

// this has the side-effect of modifying "p". that is
//   p = p \/ q
void Set_unionVoid(T p, T q);

long Set_size(T set);

int Set_equals(T set1, T set2);


#undef T

#endif
