#ifndef POLY_H
#define POLY_H

#define T Poly_t

typedef void *T;

typedef long (*Poly_tyEquals)(T, T);

typedef char *(*Poly_tyToString)(T);

typedef T (*Poly_tyId)(T);

typedef long (*tyHashCode)(T);

typedef void (*Poly_tyVoid)(T);

typedef long (*Poly_tyPred)(T);

#undef T

#endif
