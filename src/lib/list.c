#include <stdio.h>
#include <stdarg.h>
#include "assert.h"
#include "error.h"
#include "mem.h"
#include "tuple.h"
#include "list.h"


#define T List_t
#define P Poly_t

T List_new ()
{
  T l;

  Mem_NEW (l);
  l->data = 0;
  l->next = 0;
  return l;
}

static T List_new2 (P x, T l)
{
  T p;
  Mem_NEW (p);
  p->data = x;
  p->next = l;
  return p;
}

int List_isEmpty (T l)
{
  Assert_ASSERT(l);
  return (0==l->next);
}

int List_size (T l)
{
  T p;
  int i = 0;
 
  Assert_ASSERT(l);
  p = l->next;
  while (p){
    i++;
    p = p->next;
  }
  return i;
}

void List_append (T l1, T l2)
{
  List_t p;
  Assert_ASSERT(l1);
  Assert_ASSERT(l2);
  p = List_getFirst (l2);
  while (p){
    List_insertLast (l1, p->data);
    p = p->next;
  }
  return;
}

// slient for non-existing elements
void List_delete (T l, P x, Poly_tyEquals equals)
{
  List_t prev, current;
  
  Assert_ASSERT(l);
  Assert_ASSERT(x);
  Assert_ASSERT(equals);

  prev = l;
  current = l->next;
  while (current){
    if (equals (x, current->data)){
      current = current->next;
      prev->next = current;
      continue;
    }
    else;    
    prev = current;
    current = current->next;
  }
  return;
}

Poly_t List_removeHead (T l)
{
  List_t p;

  Assert_ASSERT (l);
  if (!l->next)
    Error_impossible ();

  p = l->next;
  l->next = l->next->next;
  return p->data;
}

// slient for non-existing elements
void List_deleteAll (T l, Poly_tyPred pred)
{
  List_t prev, current;
  
  Assert_ASSERT(l);
  Assert_ASSERT(pred);

  prev = l;
  current = l->next;
  while (current){
    if (pred(current->data)){
      current = current->next;
      prev->next = current;
      continue;
    }
    else;    
    prev = current;
    current = current->next;
  }
  return;
}

T List_concat (T l1, T l2)
{
  List_t result, p;

  Assert_ASSERT(l1);
  Assert_ASSERT(l2);
  
  result = List_new ();
  p = List_getFirst (l1);
  while (p){
    List_insertLast (result, p->data);
    p = p->next;
  }
  p = List_getFirst (l2);
  while (p){
    List_insertLast (result, p->data);
    p = p->next;
  }
  return result;
}

void List_appendNode (T l1, T l2)
{
  List_t tail;

  Assert_ASSERT(l1);
  Assert_ASSERT(l2);
  
  if (l1->next==0){
    l1->next = l2;
    l1->data = l2;
    l2->next = 0;
    return;
  }
  tail = (T)l1->data;
  tail->next = l2;
  l1->data = l2;
  l2->next = 0;  
  return;
}


/* we abuse the head node's "data" field to store 
 * a tail pointer 
 */
void List_insertFirst (T l, P x)
{
  T t;

  Assert_ASSERT(l);
  t = List_new2 (x, l->next);
  l->next = t;
  if (l->data == 0)
    l->data = t;
  return;
}

void List_insertLast (T l, P x)
{
  T tail, p;

  Assert_ASSERT(l);
  if (l->next == 0){
    List_insertFirst (l, x);
    return;
  }
  tail = (T)l->data;
  p = List_new2 (x, 0);
  tail->next = p;
  l->data = p;
  return;
}

T List_list (P x, ...)
{
  T list = List_new ();
  va_list ap;
  P current;

  List_insertLast (list, x);
  va_start(ap, x);
  current = va_arg(ap, P);
  while (current) {
    List_insertLast (list, current);
    current = va_arg(ap, P);
  }
  va_end(ap);  
  return list;
}

T List_rev (T l)
{
  List_t r, p;

  Assert_ASSERT(l);
  p = l->next;
  r = List_new ();
  while (p){
    List_insertFirst (r, p->data);
    p = p->next;
  }
  return r;
}

P List_nth (T l, int n)
{
  T p = l->next;
  if (n<0){
    Error_bug ("invalid argument");
    return 0;
  }
  while (p){
    if (n==0)
      return p->data;
    n--;
    p = p->next;
  }
  return 0;
}

void List_foreach (T l, void (*f)(P))
{
  T p;

  Assert_ASSERT(l);
  Assert_ASSERT(f);
  p = l->next;
  while (p){
    f (p->data);
    p = p->next;
  }
  return;
}

P List_foldl (T l, P start, P (*f)(P, P))
{
  T p;

  Assert_ASSERT(l);
  Assert_ASSERT(f);
  p = l->next;
  while (p){
    start = f (start, p->data);
    p = p->next;
  }
  return start;
}

int List_exists (T l, P x, Poly_tyEquals f)
{
  T p;

  Assert_ASSERT(l);
  Assert_ASSERT(f);
  p = l->next;
  while (p){
    if (f (x, p->data)){
      return 1;
    }
    p = p->next;
  }
  return 0;
}

int List_exists2 (T l, P x, Poly_tyEquals f, Poly_tyVoid g)
{
  T p;

  Assert_ASSERT(l);
  Assert_ASSERT(f);
  Assert_ASSERT(g);
  p = l->next;
  while (p){
    if (f (x, p->data)){
      g (p->data);
      return 1;
    }
    p = p->next;
  }
  return 0;
}

T List_map (T l, Poly_tyId f)
{
  T new, tmp;
  Assert_ASSERT(l);
  Assert_ASSERT(f);
  new = List_new ();
  tmp = l->next;
  while (tmp){
    List_insertLast (new, f(tmp->data));
    tmp = tmp->next;
  }
  return new;
}

T List_getFirst (T l)
{
  return l->next;
}

T List_filter (T l, Poly_tyPred f)
{
  List_t tmp, p;
  Assert_ASSERT(l);
  Assert_ASSERT(f);
  tmp = List_new ();
  p = List_getFirst (l);
  while (p){
    if (f(p->data))
      List_insertLast (tmp, p->data);
    p = p->next;
  }
  return tmp;
}

String_t List_toStringWithLastSep (T l,
                                   String_t sep,
                                   String_t (*f)(P))
{
  T p = List_getFirst (l);
  String_t s = "[";

  while (p){
    /* this would be rather slow, a more 
     * efficient one should be used here.
     * string buffer? or append string?
     */
    s = String_concat (s, f(p->data), sep, 0);
    p = p->next;
  }
  s = String_concat (s, "]", 0);
  return s;
}

String_t List_toString (T l,
                        String_t sep,
                        String_t (*f)(P))
{
  T p = List_getFirst (l);
  String_t s = "[";
  
  while (p){
    /* this would be rather slow, a more 
     * efficient one should be used here.
     * string buffer? or append string?
     */
    s = (p->next)? (String_concat (s, f(p->data), sep, 0))
      : (String_concat (s, f(p->data), 0));
    p = p->next;
  }
  s = String_concat (s, "]", 0);
  return s;
}

File_t List_print (T l
                   , String_t sep
                   , File_t file
                   , File_t (*f)(File_t, P))
{
  T p = List_getFirst (l);
  fprintf (file, "[");
  
  while (p){
    f (file, (p->data));
    if (p->next)
      fprintf (file, "%s", sep);
    p = p->next;
  }
  fprintf (file,  "]");
  return file;
}

#undef P
#undef T
