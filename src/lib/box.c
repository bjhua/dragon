#include <stdarg.h>
#include "assert.h"
#include "error.h"
#include "mem.h"
#include "io.h"
#include "box.h"

#define T Box_t

struct T
{
  enum{
    BOX_STR,
    /* the compiler does not like the
     * name "BOX_H".
     */
    BOX_HB,
    BOX_VB,
    BOX_INDENT
  } kind;
  union{
    String_t str;
    List_t boxes;
    struct {
      int i;
      T box;
    }indent;
  }u;
};

T Box_str (String_t str)
{
  T t;
  Mem_NEW(t);
  t->kind = BOX_STR;
  t->u.str = str;
  return t;
}

T Box_h (T x, ...)
{
  T t;
  List_t list = List_new ();
  va_list ap;
  T current;

  List_insertLast (list, x);
  va_start(ap, x);
  current = va_arg(ap, T);
  while (current) {
    List_insertLast (list, current);
    current = va_arg(ap, T);
  }
  va_end(ap);
  Mem_NEW(t);
  t->kind = BOX_HB;  
  t->u.boxes = list;
  return t;
}

T Box_hlist (List_t list)
{
  T t;
  Mem_NEW(t);
  t->kind = BOX_HB;
  t->u.boxes = list;
  return t;
}

T Box_v (T x, ...)
{
  T t;
  List_t list = List_new ();
  va_list ap;
  T current;
  
  List_insertLast (list, x);
  va_start(ap, x);
  current = va_arg(ap, T);
  while (current) {
    List_insertLast (list, current);
    current = va_arg(ap, T);
  }
  va_end(ap);
  Mem_NEW(t);
  t->kind = BOX_VB;
  t->u.boxes = list;
  return t;
}

T Box_vlist (List_t list)
{
  T t;
  Mem_NEW(t);
  t->kind = BOX_VB;
  t->u.boxes = list;
  return t;
}

T Box_indent (T b, int i)
{
  T t;
  Mem_NEW(t);
  t->kind = BOX_INDENT;
  t->u.indent.i = i;
  t->u.indent.box = b;
  return t;
}

static int Box_print2 (T b, void (*print)(String_t), 
                       int start)
{
  switch (b->kind){
  case BOX_STR:{
    print (b->u.str);
    return start+String_size (b->u.str);
  }
  case BOX_HB:{
    List_t p = List_getFirst (b->u.boxes);
    while (p){
      T b = (T)p->data;
      start += Box_print2 (b, print, start);
      if (p->next){
        print (" ");
        start++;
      }
      p = p->next;
    }
    return start;
  }
  case BOX_VB:{
    List_t p = List_getFirst (b->u.boxes);
    while (p){
      T b = (T)p->data;
      int i = Box_print2 (b, print, start);
      if (p->next){
        print ("\n");
        Io_printSpaces (start);
      }
      else
        start += i;
      p = p->next;
    }
    return start;
  }
  case BOX_INDENT:{
    int i;
    Io_printSpaces (b->u.indent.i);
    i = Box_print2 (b->u.indent.box, 
                        print, 
                        start+b->u.indent.i);
    return start+b->u.indent.i+i;
  }
  default:
    Error_bomb ();
    Error_impossible ();
    return -1;
  }
  Error_impossible ();
  return -1;
}

void Box_print (T b, void (*print)(String_t))
{
  Assert_ASSERT(b);
  Assert_ASSERT(print);

  print ("\n");
  Box_print2 (b, print, 0);
  return;
}

void Box_output (T b, File_t f)
{
  return;
}



#undef T
