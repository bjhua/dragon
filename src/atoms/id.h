#ifndef ID_H
#define ID_H

#include "../lib/string.h"
#include "../lib/property-list.h"

#define T Id_t

typedef struct T *T;

T Id_fromString (String_t s);
T Id_bogus ();
T Id_newNoName ();
int Id_hashCode (T x);
void Id_init ();
String_t Id_toString (T x);
int Id_equals (T, T);
Plist_t Id_plist (T);
void Id_print (T);

#undef T

#endif
