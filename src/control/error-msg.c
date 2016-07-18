#include <stdio.h>
#include <stdlib.h>
#include "../lib/assert.h"
#include "error-msg.h"

static int numErrors = 0;

void ErrorMsg_warn (String_t msg)
{
  fprintf (stderr,
           "Warning: %s\n",
           msg);
}

void ErrorMsg_die (String_t msg)
{
  fprintf (stderr,
           "Error: %s\n",
           msg);
  exit (0);
}

void ErrorMsg_lexError (char *s, char *file, 
                        int line, int column)
{
  numErrors++;
  fprintf (stderr, "Lexical error: (file %s, "
           "line %d, column %d)\n%s\n",             
           file, 
           line,
           column,
           s);
  exit (0);
}

void ErrorMsg_syntaxError (char *s, Region_t r)
{
  Assert_ASSERT (r);
  numErrors++;
  fprintf (stderr, "Syntax error: %s\n%s", 
           Coordinate_toString(Region_from(r)),           
           s);
  exit (0);
}

void ErrorMsg_elabError (char *s, Region_t r)
{
  Assert_ASSERT (s);
  numErrors++;
  if (r)
    fprintf (stderr, "Elaboration error: %s\n"
             "%s\n",
             Coordinate_toString (Region_from (r)),
             s);
  else 
    fprintf (stderr, "Elaboration error: \n"
             "  %s\n",
             s);
}

void ErrorMsg_init ()
{
  numErrors = 0;
}

void ErrorMsg_errorExit ()
{
  if (numErrors){
    numErrors = 0;
    exit (0);
  }
}

