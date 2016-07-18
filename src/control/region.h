#ifndef REGION_H
#define REGION_H

#include "../lib/string.h"
#include "coordinate.h"

#define T Region_t

typedef struct Region_t *Region_t;

T Region_new (Coordinate_t from, Coordinate_t to);
T Region_bogus ();
Coordinate_t Region_from (T t);
Coordinate_t Region_to (T t);
String_t Region_toString (T t);

#undef T

#endif
