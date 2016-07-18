#ifndef DOT_H
#define DOT_H

#include "string.h"
#include "file.h"
#include "poly.h"

#define T Dot_t

typedef struct T *T;

T Dot_new (Poly_tyPrint printer);
// for the "info" arg, passing 0 for non-existing ones
void Dot_insert (T d
                , Poly_t from
                , Poly_t to
                , Poly_t info);
void Dot_toJpg (T d, String_t fname);

#undef T

#endif
