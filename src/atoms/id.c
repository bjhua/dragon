#include "../lib/mem.h"
#include "../lib/hash.h"
#include "../lib/assert.h"
#include "../lib/int.h"
#include "../lib/random.h"
#include "../lib/property-list.h"
#include "id.h"

#define T Id_t

/* table: String_t -> Id_t
 */
static Hash_t table = 0;

static int counter = 0;

struct T
{
  String_t name;
  String_t newName;
  int hashCode;
  Plist_t plist;
};


static T Id_create (String_t s)
{
  T x;
  
  Assert_ASSERT(s);
  Mem_NEW (x);
  x->name = s;
  x->newName = 0;
  x->hashCode = String_hashCode (s);
  x->plist = Plist_new ();
  return x;
}

T Id_bogus ()
{
  return Id_create ("<bogus>");
}

T Id_fromString (String_t s)
{
  T x;

  Assert_ASSERT(s);
  x = Hash_lookupOrInsert (table
                           , s
                           , (tyKV)Id_create);
  return x;
}

T Id_newNoName ()
{
  T x;
  Mem_NEW (x);
  x->name = 0;
  x->newName = String_concat ("x_", 
                              Int_toString (counter++),
                              0);
  x->hashCode = Random_nextInt ();
  x->plist = Plist_new ();
  return x;
}

int Id_hashCode (T x)
{
  Assert_ASSERT(x);
  return x->hashCode;
}

String_t Id_toString (T x)
{
  Assert_ASSERT (x);
  Assert_ASSERT(((x->name==0) && (x->newName == 0)));
  return (x->name)? (x->name): (x->newName);
}

void Id_init ()
{
  table = Hash_new ((tyHashCode)String_hashCode
                    , (Poly_tyEquals)String_equals
                    // should never call this function.
                    , 0); 
}

int Id_equals (T x, T y)
{
  Assert_ASSERT(x);
  Assert_ASSERT(y);
  return x == y;
}

Plist_t Id_plist (T x)
{
  Assert_ASSERT(x);
  return x->plist;
}

void Id_print (T x)
{
  Assert_ASSERT (x);
  Assert_ASSERT((x->name==0) && (x->newName == 0));
  printf ("%s", (x->name)? (x->name): (x->newName));
  return;
}


#undef T
