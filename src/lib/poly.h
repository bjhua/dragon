#ifndef POLY_H
#define POLY_H

#include <stdbool.h>

#define T Poly_t

typedef void *T;

typedef bool (*Poly_tyEquals)(T, T);

typedef char *(*Poly_tyToString)(T);

typedef T (*Poly_tyId)(T);

typedef long (*tyHashCode)(T);

typedef void (*Poly_tyVoid)(T);

typedef bool (*Poly_tyPred)(T);

#undef T

#endif
