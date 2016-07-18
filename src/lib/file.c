#include "assert.h"
#include "error.h"
#include "file.h"

#define T File_t

int File_saveToFile (String_t fname
                     , T (*print)(T, Poly_t)
                     , Poly_t x)
{
  T file = fopen (fname, "w+");

  if (!file)
    Error_bug (String_concat 
               ("fail to open file: ",
                fname, 
                0));
  print (file, x);
  fclose (file);  
  return 0;
}

int File_flush (T f)
{
  return fflush (f);
}

T File_open (String_t s, String_t mode)
{
  T fp;
  if ((fp=fopen(s, mode)))
    return fp;

  Error_error (String_concat
               ("file open failed: "
                , s
                , " in mode "
                , "["
                , mode
                , "]"
                , 0));
  return 0;
}

void File_write (T f, String_t s)
{
  Assert_ASSERT(f);
  Assert_ASSERT(s);

  fputs (s, f);
  return;
}

void File_close (T f)
{
  int r = fclose (f);

  if (r){
    *((int *)1) = 0;
    Error_error ("close file failed\n");
  }
  return;
}


#undef T
