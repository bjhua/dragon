#include <stdio.h>
#include <stdarg.h>
#include "assert.h"
#include "string.h"
#include "mem.h"
#include "todo.h"
#include "tuple.h"

#define T Tuple_t
#define P Poly_t

struct T
{ 
  P x;
  P y;
};

T Tuple_new (P x, P y)
{
  T t;
  
  Mem_NEW (t);
  t->x = x;
  t->y = y;
  return t;
}

P Tuple_first (T t)
{
  Assert_ASSERT(t);
  return t->x;
}

P Tuple_second (T t)
{
  Assert_ASSERT(t);
  return t->y;
}

int Tuple_equals (T t1, T t2, Poly_tyEquals eqx, 
                  Poly_tyEquals eqy)
{
  Assert_ASSERT (t1);
  Assert_ASSERT (t2);
  return eqx (t1->x, t2->x) 
    && eqy (t1->y, t2->y);
}

char *Tuple_toString (T t, Poly_tyToString tx, 
                      Poly_tyToString ty)
{
  return String_concat ("(", tx(t->x), ", ", 
                        ty(t->y), ")", 0);
}

