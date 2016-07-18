#include "assert.h"
#include "mem.h"
#include "property.h"
#include "list.h"
#include "poly.h"
#include "int.h"
#include "double.h"
#include "tuple.h"

#define T Property_t
#define K Poly_t
#define V Poly_t

struct T
{
  int i;
  List_t plists;
  Plist_t (*getPlist)(Poly_t);
  V (*init)(K);
};

static int counter = 0;
static int numGets = 0;
static int numLinks = 0;
// the longest path a "get" walks
static int longestPath = 0;

T Property_new (Plist_t (*get)(K))
{
  T t;

  Mem_NEW (t);
  t->i = counter++;
  t->plists = List_new ();
  t->getPlist = get; 
  t->init = 0;
  return t;
}

T Property_newInitFun (Plist_t (*get)(K)
                       , V (*init)(K))
{
  T t;

  Mem_NEW (t);
  t->i = counter++;
  t->plists = List_new ();
  t->getPlist = get; 
  t->init = init;
  return t;
}

void Property_set (T prop, K k, V v)
{
  Tuple_t tuple;
  Plist_t plist;

  Assert_ASSERT(prop);
  Assert_ASSERT(k);
  plist = prop->getPlist (k);
  Assert_ASSERT(plist);
  tuple = Tuple_new ((Poly_t)prop, v);
  List_insertFirst (plist, tuple);
  List_insertFirst (prop->plists, plist);
  return;
}

V Property_get (T prop, K k)
{
  Plist_t plist;
  List_t p;
  int thisPath = 0;
  V v = 0;

  Assert_ASSERT(prop);
  Assert_ASSERT(k);
  plist = prop->getPlist (k);
  p = List_getFirst (plist);
  ++numGets;
  while (p){
    Tuple_t t = (Tuple_t)p->data;
  
    ++numLinks;
    ++thisPath;
    if (Tuple_first(t) == prop){
      if (thisPath > longestPath)
        longestPath = thisPath;
      
      return Tuple_second (t);
    }
    p = p->next;
  }
  
  // if the init is there, then set the new value and 
  // return it.
  if (prop->init){
    v = prop->init (k);
    
    Property_set (prop, k, v);
  }
  if (thisPath > longestPath)
    longestPath = thisPath;
  return v;
}

void Property_clear (T prop)
{
  List_t plists;
  Plist_t prev;
  Tuple_t tuple;
  
  Assert_ASSERT(prop);
  plists = List_getFirst (prop->plists);
  while (plists){
    prev = (Plist_t)plists->data;
    while (prev->next){
      tuple = (Tuple_t)prev->next->data;
      if (Tuple_first (tuple)==prop){
        prev->next = prev->next->next;
        continue;
      }
      prev = prev->next;
    }
    plists = plists->next;
  }  
}

String_t Property_status ()
{
  String_t average;
  if (numGets)
    average = Double_toString (numLinks*1.0/numGets);
  else average = "0.0";
  return String_concat 
    ("plist peeks: ", 
     Int_toString (numGets),
     ", average position: ",
     average,
     0);
}

#undef T
#undef K
#undef V
