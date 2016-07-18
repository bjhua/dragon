#include <stdio.h>
#include "assert.h"
#include "io.h"

int Io_printSpaces (int n)
{
  Assert_ASSERT(n>=0);
  while (n-->0){
    printf (" ");
  }
  return 0;
}
