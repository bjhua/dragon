#include <string.h>
#include <stdio.h>
#include "string.h"
#include "list.h"
#include "trace.h"

#define STEP 3

static int indent = 0;

static List_t names = 0;

void Trace_indent ()
{
  indent += STEP;
}

void Trace_unindent ()
{
  indent -= STEP;
}

void Trace_spaces ()
{
  int i = indent;
  while (i--)
    printf (" ");
}

int Trace_lookup (char *s)
{
  if (!names)
    names = List_new ();
  return List_exists (names, 
                            s, 
                            (Poly_tyEquals)String_equals);
}

void Trace_insert (char *s)
{
  if (!names)
    names = List_new ();
  List_insertFirst(names, s);
}

List_t Trace_allFuncs ()
{
  if (!names)
    return List_new ();
  return names;
}

void Trace_reset ()
{
  names = 0;
}

String_t Trace_junk ()
{
  return "<Trace_junk>";
}

String_t Trace_junk2 (void *s)
{
  return "<Trace_junk>";
}
