#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "string.h"
#include "poly.h"

#define T File_t

typedef FILE *T;

typedef T (*Poly_tyPrint)(T, Poly_t);

int File_flush(T);

int File_saveToFile(String_t fname, Poly_tyPrint print, Poly_t x);

T File_open(String_t fname, String_t mode);

void File_write(T, String_t s);

void File_close(T);

#undef T

#endif
