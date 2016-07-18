#ifndef APP_LIST_H
#define APP_LIST_H

#include "list.h"
#include "poly.h"

#define T AppList_t

typedef struct T *T;


// argument: List<T>
T AppList_new_list (List_t);
T AppList_new_va (T, ...);
// concatenation
T AppList_concat (T, T);

// Cook an appList from any single data item x
T AppList_fromItem (Poly_t x);
// cook an applist from any single list
// argument: List<P>
T AppList_fromItemList (List_t);
// An empty applist
T AppList_new_empty ();

List_t AppList_toList (T);

#undef T

#endif
