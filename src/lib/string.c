#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "mem.h"
#include "assert.h"
#include "string.h"
#include "file.h"

#define T String_t

T String_new (char *s)
{
  char *temp;
  int size = strlen(s) +1;
  Mem_NEW_SIZE(temp, size);
  return strcpy(temp, s);
}

T String_concat (char *s, ...)
{
  int totalSize = 0;
  char *current = s;
  char *temp, *head;

  va_list ap;
  va_start(ap, s);
  while(current) {
    totalSize += strlen (current);
    current = va_arg (ap, char *);
  }
  va_end(ap);

  
  Mem_NEW_SIZE(temp, (totalSize+1));
  head = temp;
  current = s;
  va_start(ap, s);
  while(current) {
    strcpy (temp, current);
    temp += strlen(current);
    current = va_arg (ap, char *);
  }
  return head;
}

T String_fromArray (int size, char *array[], char *sep)
{
  int i = 0;
  int totalSize = 0;
  char *temp, *head;

  while (i<size) {
    totalSize += strlen (array[i]);
    i++;
  }
  
  Mem_NEW_SIZE (temp, (totalSize + 1 + size * strlen(sep)));
  head = temp;
  i = 0;
  while (i<size) {
    strcpy (temp, array[i]);
    temp += strlen(array[i]);
    strcpy (temp, sep);
    temp += strlen (sep);
    i++;
  }
  return head;
}

int String_equals (T x, T y)
{
  return (0==strcmp(x, y));
}

String_t String_toString (T x)
{
  return x;
}

int String_size (T x)
{
  return strlen (x);
}

/* This function implements the algorithm on p.57
 * of the book "The Practice of Programming".
 */
#define MULTIPLIER 31
int String_hashCode (T x)
{
  int h = 0; 

  Assert_ASSERT(x);
  while (*x) { 
    h = h*MULTIPLIER + (unsigned)*x++;
  }
  return h;
}


void String_print (T x)
{
  Assert_ASSERT(x);
  printf ("%s", x);
}

/*
File_t String_printAsIs (File_t file, T x)
{
  int c;

  Assert_ASSERT(x);
  fprintf (file, "%s", "\"");
  while (c=*x++){
    switch (c){
    case '\n':
      fprintf (file, "%s", "\\n");
      break;
    case '\t':
      fprintf (file, "%s", "\\t");
      break;
    case '\\':
      fprintf (file, "%s", "\\");
      break;
    default:
      fprintf (file, "%c", c);
      break;
    }
  }
  fprintf (file, "%s", "\"");
}
*/

T String_toStrAsIs (T x)
{
  return x;
}

#undef BUF_SIZE

#undef T

