#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "string.h"

#define T CharBuffer_t

typedef struct T *T;

T CharBuffer_new ();
void CharBuffer_append (T x, int c);
void CharBuffer_appendString (T x, String_t s);
int CharBuffer_numItems (T x);
void CharBuffer_resetIndex (T x);
String_t CharBuffer_toString (T x);
String_t CharBuffer_toStringBeforeClear (T x);

#undef T

#endif
