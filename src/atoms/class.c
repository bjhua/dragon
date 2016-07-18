#include "../lib/mem.h"
#include "../lib/assert.h"
#include "class.h"
#include "dec.h"

#define T Class_t

T Class_new (Id_t name, List_t decs)
{
  T p;

  Mem_NEW (p);
  p->name = name;
  p->decs = decs;
  return p;
}

String_t Class_toString (T c)
{
  String_t s;
  List_t p;

  Assert_ASSERT(c);
  s = String_concat ("class ",
                     Id_toString (c->name),
                     "\n{\n",
                     0);
  p = List_getFirst (c->decs);
  while (p){
    Dec_t dec = (Dec_t)p->data;
    s = String_concat (s,
                       "  ",
                       Dec_toString (dec),
                       ";\n",
                       0);
    p = p->next;
  }
  s = String_concat (s, "}", 0);
  return s;
}

File_t Class_print (File_t file, T c)
{
  List_t p;

  Assert_ASSERT(c);
  fprintf (file, "%s", "class ");
  fprintf (file, "%s\n{\n", Id_toString (c->name));
  List_foldl(c->decs
             , file
             , (Poly_tyFold)Dec_printAsLocal);
  fprintf (file, "%s", "}\n");
  return file;
}

#undef T
