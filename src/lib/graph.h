#ifndef GRAPH_H
#define GRAPH_H

#include "poly.h"
#include "tree.h"
#include "string.h"
#include "property.h"
//#include "weight.h"
#include "list.h"
#include "set.h"


#define T Graph_t

typedef struct T *T;

T Graph_new(Poly_tyEquals equals);

T Graph_newWithName(Poly_tyEquals equals, String_t graphName);

void Graph_insertVertex(T g, Poly_t data);

void Graph_insertEdge(T g, Poly_t from, Poly_t to);

List_t Graph_successors(T, Poly_t v);

List_t Graph_predessors(T, Poly_t v);

void Graph_toJpg(T g, Poly_tyPrint printer);

void Graph_toJpgWithName(T g, Poly_tyPrint printer, String_t fname);

//void Graph_toJpgEdge (T g
//                    , String_t fname
//                    , Property_t edgeProp);

void Graph_dfs(T g, Poly_t start, void (*visit)(Poly_t));

// Tree<Poly_t>
Tree_t Graph_domTree(T g, Poly_t start);

// Each vertex's dominance frontier. Note that this
// property is set by "dfProp);
Tree_t Graph_df(T g, Poly_t start, void (*markDf)(Poly_t, Set_t));


#undef T

#endif
