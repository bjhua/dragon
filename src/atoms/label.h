#ifndef LABEL_H
#define LABEL_H

#include "../lib/string.h"
#include "../lib/property-list.h"

#define T Label_t

typedef struct T *T;

T Label_new ();
int Label_hashCode (T x);
String_t Label_toString (T x);
int Label_equals (T, T);
Plist_t Label_plist (T);
void Label_print (T);

#undef T

#endif
