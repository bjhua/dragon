#ifndef INT_H
#define INT_H

#include "string.h"

extern const int Int_zero;

String_t Int_toString(long i);

long Int_fromString(String_t s, long *result);


#endif
