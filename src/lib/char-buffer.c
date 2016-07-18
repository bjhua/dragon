#include "assert.h"
#include "mem.h"
#include "char-buffer.h"

#define T CharBuffer_t

#define INIT_SIZE 16
#define SCALE 2

struct T
{
  char *buffer;
  int last;
  int size;
};

T CharBuffer_new ()
{
  T x;

  Mem_NEW (x);
  Mem_NEW_SIZE (x->buffer, INIT_SIZE);
  x->last = 0;
  x->size = INIT_SIZE;
  return x;
}

int CharBuffer_numItems (T x)
{
  Assert_ASSERT(x);
  return x->last;
}

void CharBuffer_resetIndex (T x)
{
  Assert_ASSERT(x);
  x->last = 0;
  return;
}

void CharBuffer_append (T x, int c)
{
  int i;
  char *tmp;

  Assert_ASSERT(x);
  if (x->last >= x->size-1){
    tmp = x->buffer;
    Mem_NEW_SIZE (x->buffer, x->size*SCALE);
    x->size *= SCALE;
    for (i=0; i<x->last; i++)
      x->buffer[i] = tmp[i];
  }
  x->buffer[x->last++] = c;
  return;
}

void CharBuffer_appendString (T x, String_t s)
{
  Assert_ASSERT(x);
  Assert_ASSERT(s);
  while (*s){
    CharBuffer_append (x, *s);
    s++;
  }
  return;
}


String_t CharBuffer_toString (T x)
{
  Assert_ASSERT(x);
  x->buffer[x->last] = '\0';
  return x->buffer;
}

String_t CharBuffer_toStringBeforeClear (T x)
{
  String_t str = CharBuffer_toString (x);
  Mem_NEW_SIZE (x->buffer, INIT_SIZE);
  x->last = 0;
  x->size = INIT_SIZE;
  return str;
}

#undef T
