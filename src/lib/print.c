#include <stdio.h>

int result = 0;

static int flag = 1;
static int ind = 0;

void dprint (const char *s)
{
  printf ("to do");
  return;
}

void printIndent ()
{
  int n = ind;
  
  while (n--)
    printf (" ");
}

void indent ()
{
  ind += 3;
}

void unindent ()
{
  ind -= 3;
}

void Dp_print (char *s)
{
  if (flag)
    printf ("%s\n", s);
  else;
}
