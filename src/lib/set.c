#include "assert.h"
#include "mem.h"
#include "list.h"
#include "set.h"

#define T Set_t

struct T
{
  Poly_tyEquals equals;
  // use a very slow list-based set representation for now
  List_t list;
};

T Set_new (Poly_tyEquals equals)
{
  T set;
  
  Mem_NEW(set);
  set->equals = equals;
  set->list = List_new ();
  return set;
}

int Set_exists (T set, Poly_t x)
{
  List_t p;

  Assert_ASSERT(set);

  p = List_getFirst (set->list);
  while (p){
    if (set->equals (x, p->data))
      return 1;
    p = p->next;
  }
  return 0;
}

void Set_delete (T set, Poly_t x)
{
  List_t p;

  Assert_ASSERT(set);

  List_delete (set->list, x, set->equals);
  return;
}

// delete all items x for pred(x) is true
void Set_deleteAll (T set, Poly_tyPred pred)
{
  List_t p;

  Assert_ASSERT(set);

  List_deleteAll (set->list, pred);
  return;
}

void Set_foreach (T set, Poly_tyVoid f)
{
  Assert_ASSERT(set);

  List_foreach (set->list, f);
  return;
}

int Set_isEmpty (T set)
{
  Assert_ASSERT(set);

  return List_isEmpty (set->list);
}

// Remove one element from a set.
Poly_t Set_removeOne (T set)
{
  Assert_ASSERT(set);

  if (Set_isEmpty (set))
    Error_impossible ();

  return List_removeHead (set->list);
}

void Set_insert (T set, Poly_t x)
{
  if (Set_exists (set, x))
    return;

  List_insertLast (set->list, x);
  return;
}

long Set_size (T set)
{
  Assert_ASSERT(set);

  return List_size (set->list);
}

T Set_fromList (Poly_tyEquals equals, List_t l)
{
  T set;
  
  Mem_NEW(set);
  set->equals = equals;
  set->list = l;
  return set;
}

T Set_singleton (Poly_tyEquals equals, Poly_t x)
{
  T set;
  
  Mem_NEW(set);
  set->equals = equals;
  set->list = List_list (x, 0);
  return set;
}

List_t Set_toList (T set)
{
  Assert_ASSERT(set);

  return (set->list);
}

T Set_intersection (T set1, T set2)
{
  T newSet;
  List_t p;

  Assert_ASSERT(set1);
  Assert_ASSERT(set2);

  newSet = Set_new (set1->equals);
  p = List_getFirst (set1->list);
  while (p){
    Poly_t v = (Poly_t)p->data;

    if (List_exists (set2->list, v, set1->equals))
      Set_insert (newSet, v);
    else;
    p = p->next;
  }
  return newSet;
}

T Set_union (T set1, T set2)
{
  T newSet;
  List_t p;

  Assert_ASSERT(set1);
  Assert_ASSERT(set2);

  newSet = Set_new (set1->equals);
  p = List_getFirst (set1->list);
  while (p){
    Set_insert (newSet, p->data);
    p = p->next;
  }
  p = List_getFirst (set2->list);
  while (p){
    Poly_t v = (Poly_t)p->data;

    if (!List_exists (set1->list, v, set1->equals))
      Set_insert (newSet, v);
    else;
    p = p->next;
  }
  return newSet;
}

void Set_unionVoid (T set1, T set2)
{
  List_t p;

  Assert_ASSERT(set1);
  Assert_ASSERT(set2);

  p = List_getFirst (set2->list);
  while (p){
    Poly_t v = (Poly_t)p->data;

    // this may be further enhanced
    if (!List_exists (set1->list, v, set1->equals))
      Set_insert (set1, v);
    else;
    p = p->next;
  }
  return;
}

int Set_equals (T set1, T set2)
{
  T newSet;
  List_t p;

  Assert_ASSERT(set1);
  Assert_ASSERT(set1);

  if (List_size (set1->list) != List_size (set2->list))
    return 0;

  p = List_getFirst (set1->list);
  while (p){
    Poly_t v = (Poly_t)p->data;

    if (List_exists (set2->list, v, set1->equals))
      ; 
    else
      return 0;
    p = p->next;
  }  
  return 1;
}


#undef T

