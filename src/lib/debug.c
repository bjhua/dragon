#include <stdio.h>
#include "debug.h"


#ifndef NDEBUG
#define DEBUG_FLAG 1 
#else
#define DEBUG_FLAG 0
#endif


void Debug_printInt (int i)
{
  if (DEBUG_FLAG)
    printf ("%d\n", i);
  else;
}
