#ifndef LIST_H
#define LIST_H

#include "file.h"
#include "string.h"
#include "poly.h"

#define T List_t
#define P Poly_t

typedef struct T *T;
struct T
{
  Poly_t data;
  List_t next;
};

typedef P (*Poly_tyFold)(P, P);
typedef File_t (*Poly_tyListPrint)(File_t, P);

T List_new ();
T List_concat (T, T);
int List_isEmpty (T l);
int List_size (T l);
void List_insertFirst (T l, P x);
void List_insertLast (T l, P x);
void List_foreach (T l, void (*f) (P));
void List_append (T l1, T l2);
void List_appendNode (T l1, T l2);
void List_delete (T l1, P x, Poly_tyEquals equals);
void List_deleteAll (T, Poly_tyPred pred);
T List_map (T l, Poly_tyId f);
T List_list (P x, ...);
P List_removeHead (T l);
P List_nth (T l, int n);
// f (f (f (starter, x1), x2), ...)
P List_foldl (T list
              , P starter
              , P(*f)(P, P));
T List_rev (T l);
int List_exists (T l, P x, Poly_tyEquals f);
/* find the first element y, which satisfied f(x, y), and 
 * apply g(y) before return.
 */
int List_exists2 (T l, P x, 
                  Poly_tyEquals f, 
                  Poly_tyVoid g);
T List_filter (T l, Poly_tyPred f);
T List_getFirst (T l);
String_t List_toString (T list, 
                        String_t sep, 
                        String_t (*f)(P));
String_t List_toStringNoLastSep (T list, 
                                 String_t sep, 
                                 String_t (*f)(P));
// print out "list" to "file", with separator "sep",
// "sep" does not appear last, each element is print out
// by "f".
File_t List_print (T list
                   , String_t sep
                   , File_t file
                   , File_t (*f)(File_t, Poly_t));

#undef P
#undef T

#endif
