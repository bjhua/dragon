#ifndef COORDINATE_H
#define COORDINATE_H

#include "../lib/string.h"

#define T Coordinate_t

typedef struct Coordinate_t *Coordinate_t;

T Coordinate_new (String_t file, int line, int column);
T Coordinate_bogus ();
String_t Coordinate_file (T t);
int Coordinate_column (T t);
int Coordinate_line (T t);
String_t Coordinate_toString (T t);

#undef T

#endif
