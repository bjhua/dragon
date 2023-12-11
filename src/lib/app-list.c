#include "app-list.h"
#include "mem.h"
#include <assert.h>

#define T AppList_t

struct T {
    enum {
        EMPTY,
        SINGLE,
        APP,
        LIST
    } kind;
    union {
        // List<P>
        List_t single;
        struct {
            T left;
            T right;
        } app;
        // List<T>
        List_t list;
    } u;
};


T AppList_new_empty() {
    T t;

    Mem_NEW(t);
    t->kind = EMPTY;
    return t;
}

T AppList_fromItem(Poly_t x) {
    T t;

    Mem_NEW(t);
    t->kind = SINGLE;
    t->u.list = List_list(x, 0);
    return t;
}

//static T AppList_new_fromItemList(List_t x) {
//    T t;
//
//    Mem_NEW(t);
//    t->kind = SINGLE;
//    t->u.list = x;
//    return t;
//}

T AppList_concat(T x, T y) {
    T t;

    Mem_NEW(t);
    t->kind = APP;
    t->u.app.left = x;
    t->u.app.right = y;
    return t;
}

T AppList_new_list(List_t x) {
    T t;

    Mem_NEW(t);
    t->kind = LIST;
    t->u.list = x;
    return t;
}

/*
T AppList_new_va (T x, ...)
{
  T t;
  va_list ap;
  T current;
  List_t tmp = List_new ();

  Mem_NEW(t);
  t->kind = LIST;

  List_insertLast (tmp, x);
  va_start(ap, x);
  current = va_arg(ap, T);
  while (current) {
    List_insertLast (tmp, current);
    current = va_arg(ap, T);
  }
  va_end(ap);  
  t->u.list = tmp;
  return t;
}

*/

static List_t theList = 0;

static void toList_doit(T x) {
    switch (x->kind) {
        case EMPTY:
            break;
        case SINGLE:
            List_append(theList, x->u.single);
            break;
        case APP:
            toList_doit(x->u.app.left);
            toList_doit(x->u.app.right);
            break;
        case LIST: {
            List_t tmp = List_getFirst(x->u.list);

            while (tmp) {
                T p = (T) tmp->data;
                toList_doit(p);
                tmp = tmp->next;
            }
            break;
        }
        default:
            Error_impossible();
            return;
    }
    return;
}

List_t AppList_toList(T x) {
    assert(x);

    theList = List_new();
    toList_doit(x);
    return theList;
}


#undef T
