#ifndef AST_ID_H
#define AST_ID_H

#include "../lib/string.h"
#include "../lib/property-list.h"
#include "../atoms/id.h"
#include "../control/region.h"

#define T AstId_t

typedef struct T *T;

T AstId_fromString (String_t s, Region_t r);
T AstId_bogus ();
int AstId_equals (T id1, T id2);
int AstId_hashCode (T);
T AstId_newNoName ();
String_t AstId_toString (T);
Id_t AstId_toId (T);
Plist_t AstId_plist (T);
Region_t AstId_dest (T);
Region_t AstId_getRegion (T);
void AstId_print (T);

#undef T

#endif
