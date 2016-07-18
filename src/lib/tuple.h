#ifndef TUPLE_H
#define TUPLE_H

#include "poly.h"

#define T Tuple_t
#define P Poly_t

typedef struct T *T;

T Tuple_new (P x, P y);
P Tuple_first (T t); 
P Tuple_second (T t);
int Tuple_equals (T t1, T t2, Poly_tyEquals eqx, 
                  Poly_tyEquals eqy);
char *Tuple_toString (T t, Poly_tyToString tx, 
                      Poly_tyToString ty);

#undef P
#undef T

#endif
