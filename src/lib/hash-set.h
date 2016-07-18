#ifndef HASH_SET_H
#define HASH_SET_H

#include "poly.h"
#include "set.h"
#include "list.h"

#define T HashSet_t

typedef struct T *T;

T HashSet_new (int (*)(Poly_t)
               , Poly_tyEquals equals);
void HashSet_delete (T set, Poly_t x);
int HashSet_exists (T set, Poly_t x);
T HashSet_fromList (Poly_tyEquals equals, List_t list);
void HashSet_foreach (T set, Poly_tyVoid f);
void HashSet_insert (T set, Poly_t x);
T HashSet_intersection (T, T);
int HashSet_isEmpty (T);
// Remove one element from a set.
Poly_t HashSet_removeOne (T);
T HashSet_singleton (Poly_tyEquals equals, Poly_t x);
List_t HashSet_toList (T set);
T HashSet_union (T, T);
// this has the side-effect of modifying "p". that is
//   p = p \/ q
void HashSet_unionVoid (T p, T q);
// this has the side-effect of modifying "p". that is
//   p = p \/ q
void HashSet_unionVoidSet (T p, Set_t q);
// turn a hashset to set.
Set_t HashSet_toSet (Poly_tyEquals eqs, T set);

#undef T

#endif
