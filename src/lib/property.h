#ifndef PROPERTY_H
#define PROPERTY_H

#include "poly.h"
#include "string.h"
#include "property-list.h"

#define T Property_t
#define K Poly_t
#define V Poly_t
#define P Plist_t

typedef struct T *T;

typedef V(*Poly_tyPropInit)(K);

T Property_new(Poly_tyPlist);
// take an extra "init" argument, which will be called
// when the search failed on some item "k", and the 
// generated
// "V" will be set on that item "k".
T Property_newInitFun(Poly_tyPlist, V(*init)(K));

void Property_set(T prop, K k, V v);

V Property_get(T, K k);

void Property_clear(T prop);

String_t Property_status();

#undef T
#undef K
#undef V
#undef P

#endif

