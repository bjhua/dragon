#include "property-list.h"

#define T Plist_t

T Plist_new ()
{
  return List_new ();
}

int Plist_equals (T p1, T p2)
{
  return p1 == p2;
}

#undef T
