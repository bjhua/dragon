#ifndef POLY_H
#define POLY_H

#define T Poly_t

typedef void *T;

typedef int (*Poly_tyEquals)(T, T);
typedef char *(*Poly_tyToString) (T);
typedef T (*Poly_tyId)(T);
typedef int (*tyHashCode)(T);
typedef void (*Poly_tyVoid)(T);
typedef int (*Poly_tyPred)(T);

#undef T

#endif
