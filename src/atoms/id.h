#ifndef ID_H
#define ID_H

#include "../lib/property-list.h"
#include "../lib/string.h"

#define T Id_t

typedef struct T *T;

T Id_fromString(String_t s);
T Id_bogus(void);
T Id_newNoName(void);
long Id_hashCode(T x);
void Id_init(void);
String_t Id_toString(T x);
long Id_equals(T, T);
Plist_t Id_plist(T);
void Id_print(T);

#undef T

#endif
