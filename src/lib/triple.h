#ifndef TRIPLE_H
#define TRIPLE_H

#include "poly.h"

#define T Triple_t
#define P Poly_t

typedef struct T *T;

T Triple_new (P x, P y, P z);
P Triple_first (T t); 
P Triple_second (T t);
P Triple_third (T t);
int Triple_equals (T t1, T t2, 
                   Poly_tyEquals eqx, 
                   Poly_tyEquals eqy,
                   Poly_tyEquals eqz);
char *Triple_toString (T t, 
                       Poly_tyToString tx, 
                       Poly_tyToString ty,
                       Poly_tyToString tz);

#undef P
#undef T

#endif
