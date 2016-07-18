#ifndef INT_H
#define INT_H

#include "string.h"

extern const int Int_zero;

String_t Int_toString (int i);
int Int_fromString (String_t s, int *result);


#endif
