#ifndef PROPERTY_LIST_H
#define PROPERTY_LIST_H

#include "list.h"
#include "poly.h"

#define T Plist_t

typedef List_t T;

typedef T (*Poly_tyPlist)(Poly_t);

T Plist_new();
/* int PropertyList_equals (T, T); */

#undef T

#endif
