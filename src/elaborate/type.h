#ifndef TYPE_H
#define TYPE_H

#include "../lib/list.h"
#include "../lib/string.h"
#include "../lib/tuple.h"
#include "../ast/ast-id.h"

#define T Type_t

typedef struct T *T;
struct T
{
  enum {
    TYPE_A_INT, 
    TYPE_A_STRING,
    TYPE_A_NS,
    TYPE_A_CLASS, 
    TYPE_A_PRODUCT,
    TYPE_A_FUN
  }kind;
  int isArray;
  union{
    AstId_t className;
    /* List<T> */
    List_t product;
    struct{
      T from;
      T to;
    }fun;
  }u;
};

T Type_new_int ();
T Type_new_string ();
T Type_new_ns ();
T Type_new_class (AstId_t name);
T Type_new_array (T);
T Type_new_product (T t, ...);
T Type_new_product2 (List_t prod);
T Type_new_fun (T arg, T ret);
void Type_set_array (T);
Id_t Type_dest_array (T);
T Type_clearArray (T);
Id_t Type_searchField (T t, AstId_t id);
Tuple_t Type_dest_fun (T);
int Type_equals (T, T);
int Type_equals_int (T);
int Type_equals_string (T);
String_t Type_toString (T);

#undef T

#endif
