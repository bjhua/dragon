#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void Error_error (const char *s)
{
  fprintf (stderr, "Error: %s\n", s);
  exit (1);
}

void Error_error2 (const char *s, const char *s2)
{
  fprintf (stderr, "Error: %s %s\n", s, s2);
  exit (1);
}

void Error_fatal (const char *s)
{
  fprintf (stderr, "Error: %s\n", s);
  exit (1);
}
