#include <stdio.h>
#include <stdarg.h>
#include "assert.h"
#include "string.h"
#include "mem.h"
#include "todo.h"
#include "triple.h"

#define T Triple_t
#define P Poly_t

struct T
{ 
  P x;
  P y;
  P z;
};

T Triple_new (P x, P y, P z)
{
  T t;
  
  Mem_NEW (t);
  t->x = x;
  t->y = y;
  t->z = z;
  return t;
}

P Triple_first (T t)
{
  Assert_ASSERT(t);
  return t->x;
}

P Triple_second (T t)
{
  Assert_ASSERT(t);
  return t->y;
}

P Triple_third (T t)
{
  Assert_ASSERT(t);
  return t->z;
}

int Triple_equals (T t1, T t2, Poly_tyEquals eqx, 
                   Poly_tyEquals eqy,
                   Poly_tyEquals eqz)
{
  Assert_ASSERT (t1);
  Assert_ASSERT (t2);
  return eqx (t1->x, t2->x) 
    && eqy (t1->y, t2->y)
    && eqz (t1->z, t2->z);
}

char *Triple_toString (T t, Poly_tyToString tx, 
                       Poly_tyToString ty,
                       Poly_tyToString tz)
{
  return String_concat ("(", 
                        tx(t->x), 
                        ", ", 
                        ty(t->y), 
                        ", ",
                        tz (t->z),
                        ")", 
                        0);
}

#undef P
#undef T
