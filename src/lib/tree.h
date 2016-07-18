// This is a general tree module: each vertex
// may have an arbitary number of children.
#ifndef TREE_H
#define TREE_H

#include "poly.h"
#include "string.h"
#include "list.h"

#define T Tree_t

typedef struct T *T;

T Tree_new (Poly_tyEquals equals);
T Tree_newWithName (Poly_tyEquals equals
                    , String_t name);
void Tree_insertVertex (T t, Poly_t data);
void Tree_insertEdge (T t
                      , Poly_t from
                      , Poly_t to);
void Tree_toJpg (T t
                 , Poly_tyPrint printer);
void Tree_toJpgWithName (T t
                         , Poly_tyPrint printer
                         , String_t fname);
void Tree_dfs (T t
               , Poly_t start
               , void (*visit)(Poly_t));
// List<P>
List_t Tree_children (T t, Poly_t n);

Tree_t Tree_map (T t, Poly_tyEquals, Poly_t(*map)(Poly_t));

#undef T

#endif
