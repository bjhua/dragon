#include <stdio.h>
#include "assert.h"
#include "mem.h"
#include "property-list.h"
#include "property.h"
#include "list.h"
#include "error.h"
#include "dot.h"
#include "set.h"
#include "../ssa/ssa.h"
#include "graph.h"

// 
#ifdef printf
#undef printf
#endif
#define printf noop
#define dprintf noop

static int noop(char *s, ...) {
    return 0;
}

struct Ssa_Block_t;

#define T Graph_t

struct T {
    String_t name;
    Poly_tyEquals equals;
    // List<V>
    List_t vs;
};

#define V Vertex_t
#define E Edge_t

typedef struct V *V;
typedef struct E *E;

///////////////////////////////////////////////////////
// vertex
struct V {
    Poly_t data;
    // List<E>
    List_t edges;
    Plist_t plist;
};

static V Vertex_new(Poly_t data) {
    V v;

    Mem_NEW(v);
    v->data = data;
    v->edges = List_new();
    v->plist = Plist_new();
    return v;
}

static Plist_t Vertex_plist(V v) {
    Assert_ASSERT(v);

    return v->plist;
}

static int Vertex_equals(V v1, V v2) {
    Assert_ASSERT(v1);
    Assert_ASSERT(v2);

    return v1 == v2;
}

////////////////////////////////////////////////////////
// edge
struct E {
    V from;
    V to;
    Plist_t plist;
};

static E Edge_new(V from, V to) {
    E e;

    Mem_NEW(e);
    e->from = from;
    e->to = to;
    e->plist = Plist_new();
    return e;
}

static E Edge_fromData(Poly_t from, Poly_t to) {
    E e;

    Error_impossible ();
    return e;
}


static Plist_t Edge_plist(E e) {
    Assert_ASSERT(e);

    return e->plist;
}


/////////////////////////////////////////////////////
// graph
T Graph_new(Poly_tyEquals eq) {
    T g;

    Mem_NEW(g);
    g->name = "NONE";
    g->equals = eq;
    g->vs = List_new();
    return g;
}

T Graph_newWithName(Poly_tyEquals eq, String_t name) {
    T g;

    Mem_NEW(g);
    g->name = name;
    g->equals = eq;
    g->vs = List_new();
    return g;
}

static V searchVertex(T g, Poly_t data) {
    List_t p;
    Poly_tyEquals equals;

    Assert_ASSERT(g);
    Assert_ASSERT(data);

    p = List_getFirst(g->vs);
    equals = g->equals;
    while (p) {
        V v = (V) (p->data);

        if (equals(data, v->data))
            return v;
        p = p->next;
    }
    Error_error("vertex not found: graph.c\n");
    return 0;
}

static E searchEdge(T g, Poly_t from, Poly_t to) {
    V fv = searchVertex(g, from);
    List_t edges = List_getFirst(fv->edges);
    while (edges) {
        E current = (E) (edges->data);
        V currentTo = current->to;
        if (g->equals(to, currentTo->data))
            return current;
        edges = edges->next;
    }
    Error_error("no this edge: searchEdge: graph.c\n");
    return 0;
}

/////////////////////////////////////////////////////
// 
void Graph_insertVertex(T g, Poly_t x) {
    V v = Vertex_new(x);
    List_insertLast(g->vs, v);
    return;
}

void Graph_insertEdge(T g, Poly_t from, Poly_t to) {
    V fv = searchVertex(g, from);
    V tv = searchVertex(g, to);
    E e = Edge_new(fv, tv);
    List_insertLast(fv->edges, e);
    return;
}

void Graph_visitAllVertex(T g, void (*visit)(Poly_t)) {
    List_t l = List_getFirst(g->vs);

    while (l) {
        V v = (V) l->data;

        visit(v->data);
        l = l->next;
    }
    return;
}

void Graph_toJpgWithName(T g, Poly_tyPrint printer, String_t fname) {
    Dot_t d = Dot_new(printer);
    String_t name = String_concat(g->name, fname, 0);
    List_t vs = List_getFirst(g->vs);

    while (vs) {
        V v = (V) (vs->data);
        Poly_t sfrom = v->data;
        List_t es = List_getFirst(v->edges);

        while (es) {
            E e = (E) (es->data);
            Poly_t sto = e->to->data;
            Dot_insert(d, sfrom, sto, 0);
            es = es->next;
        }
        vs = vs->next;
    }
    Dot_toJpg(d, name);
}

void Graph_toJpg(T g, Poly_tyPrint printer) {
    Graph_toJpgWithName(g, printer, "test");
}

////////////////////////////////////////////////////
// dfs
static void Graph_dfsDoit(T g, V v, Poly_tyVoid f, Property_t visited) {
    List_t edges;

    f(v->data);
    Property_set(visited, v, (Poly_t) 1);
    edges = List_getFirst(v->edges);
    while (edges) {
        E e = (E) edges->data;
        V to = e->to;
        Poly_t visitedTo = Property_get(visited, to);
        if (!visitedTo) {
            Graph_dfsDoit(g, to, f, visited);
        } else;
        edges = edges->next;
    }
    return;
}

void Graph_dfs(T g, Poly_t start, Poly_tyVoid f) {
    V sv;
    Property_t visited;

    Assert_ASSERT(g);
    Assert_ASSERT(start);
    Assert_ASSERT(f);

    sv = searchVertex(g, start);
    visited = Property_new((Poly_tyPlist) Vertex_plist);
    if (sv)
        Graph_dfsDoit(g, sv, f, visited);

    // don't visit nodes unreachable from start
    // may add a flag to control this

    Property_clear(visited);
    return;
}

////////////////////////////////////////////////////
// dominator tree-related
static void markPreds(T g, Property_t preds) {
    Assert_ASSERT(g);

    List_t vs = List_getFirst(g->vs);
    while (vs) {
        V from = (V) vs->data;
        List_t edges = List_getFirst(from->edges);
        while (edges) {
            E e = (E) edges->data;
            V to = e->to;

            // "from" is a predecesor for "to"
            Set_t set = Property_get(preds, to);
            if (set) {
                Set_insert(set, from);
            } else {
                Set_t newSet
                        = Set_new((Poly_tyEquals) Vertex_equals);
                Set_insert(newSet, from);
                Property_set(preds, to, newSet);
            }
            edges = edges->next;
        }
        vs = vs->next;
    }
    return;
}

static void predDoit(V v) {
    printf("%s ", v->data);
    return;
}

static void printPreds(T g, Property_t preds) {
    Assert_ASSERT(g);

    List_t vs = List_getFirst(g->vs);
    while (vs) {
        V from = (V) vs->data;
        Set_t set = Property_get(preds, from);
        // hard-coded here
        printf("%s ==>", from->data);
        if (set) {
            printf("(%d) ", Set_size(set));
            Set_foreach(set, (Poly_tyVoid) predDoit);
        } else
            printf("[]");
        printf("\n");
        vs = vs->next;
    }
    return;
}

static void printOneVertex(V v) {
    dprintf("%s, ", Label_toString(((Ssa_Block_t) (v->data))->label));
}

static void printDoms(T g, Property_t domProp) {
    Assert_ASSERT(g);

    List_t vs = List_getFirst(g->vs);
    while (vs) {
        V from = (V) vs->data;
        Set_t set = Property_get(domProp, from);
        // hard-coded here
        dprintf("%s ==>", Label_toString(((Ssa_Block_t) (from->data))->label));
        if (!set)
            Error_impossible ();

        dprintf("(%d doms) [", Set_size(set));
        Set_foreach(set, (Poly_tyVoid) printOneVertex);
        dprintf("]\n");
        vs = vs->next;
    }
    return;
}

static void initDom(T g, V start, Property_t dom) {
    List_t vs = List_getFirst(g->vs);

    while (vs) {
        V v = (V) vs->data;

        Property_set(dom, v, Set_fromList
                ((Poly_tyEquals) Vertex_equals, g->vs));
        vs = vs->next;
    }
    Property_set(dom, start, Set_singleton((Poly_tyEquals) Vertex_equals, start));
    return;
}

static Set_t idomPropInitFun(V v) {
    return Set_new((Poly_tyEquals) Vertex_equals);
}

static Property_t globalDeletable = 0;

static long idomDeletable(V v) {
    return (long) Property_get(globalDeletable, v);
}

// This algorithm is based on Figure 7-14 of Muchnick.

// Calculate dominator tree. 
// The returned tree consists of graph vertex.
// If dom!=0, return dominace property
// If idom!=0, return the
// immediate dominator in "idom" (it is on the 
// graph vertex, not tree vertex).
static Tree_t Graph_domTreeReal(T g, Poly_t start, Property_t *domProp, Property_t *idomProp) {
    // Tree<Vertex_t>
    Tree_t tree;
    // calculate the predessors for each vertex
    // V -> Set<V>
    Property_t preds;
    // V -> Set<V>, may not be empty
    Property_t dom;
    // V -> Set<V>, may be empty
    Property_t idom;

    V startv;
    int changed;

    startv = searchVertex(g, start);

    preds = Property_new((Poly_tyPlist) Vertex_plist);
    markPreds(g, preds);
    //now, every vertex should have predessors
    //printPreds (g, preds);

    // V -> dom (a set of vertex)
    dom = Property_new((Poly_tyPlist) Vertex_plist);
    // init dom for all vertex
    initDom(g, startv, dom);
    //now, every vertex should have initial dominators
    printf("initialized dominators\n");
    //printPreds (g, dom);

    idom = Property_newInitFun((Poly_tyPlist) Vertex_plist, (Poly_tyPropInit) idomPropInitFun);

    // fix-point algorithm. later, this should be changed
    // to a more efficient algorithm such as bitarray-based
    changed = 1;
    while (changed) {
        List_t vs = List_getFirst(g->vs);
        changed = 0;

        while (vs) {
            V current = (V) vs->data;
            List_t predList;
            Set_t predSet;
            // this will hold the final result
            Set_t result = Set_new((Poly_tyEquals) Vertex_equals);
            Set_t domSetCurrent = Property_get(dom, current);
            // should we require this?
            result = Set_union(result, domSetCurrent);

            // as the dom set has been properly initialized, so
            // it must not be empty.
            if (!domSetCurrent)
                Error_impossible ();
            // the entry vertex should not be caculated
            if (Vertex_equals(current, startv)) {
                vs = vs->next;
                continue;
            }
            // this is a normal vertex. Every vertex should
            // have predessor set, except for entry vertex.
            predSet = (Set_t) Property_get(preds, current);
            if (!predSet)
                Error_impossible ();

            // /\ pred_p dom(p)
            predList = List_getFirst(Set_toList(predSet));
            while (predList) {
                V p = (V) predList->data;
                Set_t domforP = Property_get(dom, p);

                printf("looping preds\n");
                result = Set_intersection(result, domforP);
                predList = predList->next;
            }
            Set_insert(result, current);
            // changed?
            if (!Set_equals(result, domSetCurrent)) {
                changed = 1;
                Property_set(dom, current, result);
            }
            vs = vs->next;
        }

    }// end of outer loop
    // now, the dom set should be ready
    printf("dominators finished\n");
    printDoms(g, dom);

    // construct the dominator tree
    {
        List_t vs = List_getFirst(g->vs);
        // which vertex is the deleta candidate from idom
        Property_t deletable
                = Property_new((Poly_tyPlist) Vertex_plist);

        // init
        while (vs) {
            V current = (V) vs->data;
            Set_t domSet = (Set_t) Property_get(dom, current);
            Set_t idomSet
                    = (Set_t) Set_new((Poly_tyEquals) Vertex_equals);

            idomSet = Set_union(idomSet, domSet);
            Set_delete(idomSet, current);
            Property_set(idom, current, idomSet);
            vs = vs->next;
        }
        printf("sub-dominators finished\n");
        printDoms(g, idom);

        //
        vs = List_getFirst(g->vs);
        while (vs) {
            V current = (V) vs->data;
            Set_t idomSet = (Set_t) Property_get(idom, current);
            List_t idomList
                    = List_getFirst(Set_toList(idomSet));
            List_t loop = idomList;
            List_t nest = idomList;

            while (loop) {
                V x = (V) loop->data;
                nest = idomList;

                while (nest) {
                    V y = (V) nest->data;
                    Set_t domfory = Property_get(dom, y);

                    if (Vertex_equals(x, y)) {
                        nest = nest->next;
                        continue;
                    } else;
                    if (Set_exists(domfory, x))
                        Property_set(deletable, x, (Poly_t) 1);
                    else;
                    nest = nest->next;
                }
                loop = loop->next;
            }
            globalDeletable = deletable;
            Set_deleteAll(idomSet, (Poly_tyPred) idomDeletable);
            Property_clear(deletable);
            vs = vs->next;
        }
    } // local scope

    printf("i-dominators finished\n");
    printDoms(g, idom);

    Property_clear(preds);

    // contruct tree
    {
        List_t vs = List_getFirst(g->vs);

        tree = Tree_newWithName((Poly_tyEquals) Vertex_equals, String_concat(g->name, "domTree", 0));

        // insert all graph vertex
        while (vs) {
            V v = (V) vs->data;

            // note that we should insert the data, not vertex
            Tree_insertVertex(tree, v);
            vs = vs->next;
        }

        // insert domination edges
        vs = List_getFirst(g->vs);
        while (vs) {
            V v = (V) vs->data;
            Set_t idomSet = Property_get(idom, v);
            List_t idomList
                    = List_getFirst(Set_toList(idomSet));
            int num = 0;
            while (idomList) {
                V from = (V) idomList->data;

                Tree_insertEdge(tree, from, v);
                num++;
                idomList = idomList->next;
            }
            // if more that one idom, it must be our error.
            if (num > 1)
                Error_impossible ();

            vs = vs->next;
        }
    }

    if (idomProp)
        *idomProp = idom;
    else
        Property_clear(idom);

    if (domProp)
        *domProp = dom;
    else
        Property_clear(dom);

    return tree;
}



//////////////////////////////////////////////////////
// dominator frontier

// Return: Set<V>
static Set_t Graph_dfDoit(T g, V n, Tree_t domTree, Property_t dom, Property_t idom, void (*markDf)(Poly_t, Set_t)) {
    // Set<V>
    Set_t dfSet = Set_new((Poly_tyEquals) Vertex_equals);
    Set_t markSet = Set_new((Poly_tyEquals) g->equals);

    List_t nedges = List_getFirst(n->edges);
    // List<V>
    List_t children
            = List_getFirst(Tree_children(domTree, n));

    while (nedges) {
        E e = nedges->data;
        V y = e->to;
        Set_t idomy = Property_get(idom, y);

        // the idom set is empty
        if (Set_size(idomy) == 0)
            Set_insert(dfSet, y);

        if (Set_size(idomy) > 1)
            Error_impossible ();


        if (!Set_exists(idomy, n))
            Set_insert(dfSet, y);

        nedges = nedges->next;
    }

    while (children) {
        V c = (V) children->data;
        Set_t dfOfc
                = Graph_dfDoit(g, c, domTree, dom, idom, markDf);
        List_t dfOfcList = List_getFirst(Set_toList(dfOfc));
        while (dfOfcList) {
            V w = (V) dfOfcList->data;
            Set_t domofw = Property_get(dom, w);
            if (!domofw)
                Error_impossible ();
            // if n does not dominate w, or if n==w
            if (!Set_exists(domofw, n)
                || Vertex_equals(n, w))
                Set_insert(dfSet, w);

            dfOfcList = dfOfcList->next;
        }
        children = children->next;
    }
    // mark df set on data
    printf("%s ==> [", Label_toString(((Ssa_Block_t) (n->data))->label));
    {
        List_t l = List_getFirst(Set_toList(dfSet));
        while (l) {
            V v = (V) l->data;
            Label_t label = ((Ssa_Block_t) (v->data))->label;
            printf("%s ", Label_toString(label));
            Set_insert(markSet, v->data);
            l = l->next;
        }
        printf("]\n");
        markDf(n->data, markSet);
    }
    return dfSet;
}

static Poly_t localMap(V v) {
    return v->data;
}

// mark each vertex df, when markDf!=0
Tree_t Graph_df(T g, Poly_t start, void (*markDf)(Poly_t, Set_t)) {
    // V -> Set_t<V>
    Property_t domProp
            = Property_new((Poly_tyPlist) Vertex_plist);
    // V -> Set_t<V>, may be empty
    Property_t idomProp
            = Property_new((Poly_tyPlist) Vertex_plist);
    // Tree<V>
    Tree_t domTree = Graph_domTreeReal(g, start, &domProp, &idomProp);


    V startv = searchVertex(g, start);

    // checking doms
    dprintf("checking doms:\n");
    printDoms(g, domProp);
    dprintf("checking idoms:\n");
    printDoms(g, idomProp);

    dprintf("Graph_df starting:\n");
    Graph_dfDoit(g, startv, domTree, domProp, idomProp, markDf);


    Property_clear(domProp);
    Property_clear(idomProp);
    dprintf("Graph_df finished\n");
    return Tree_map(domTree, g->equals, (Poly_tyId) localMap);
}

Tree_t Graph_domTree(T g, Poly_t start) {
    V startv = searchVertex(g, start);
    Tree_t domTree = Graph_domTreeReal(g, startv, 0, 0);
    return Tree_map(domTree, g->equals, (Poly_tyId) localMap);
}

List_t Graph_successors(T g, Poly_t k) {
    List_t result = List_new();
    V v = searchVertex(g, k);
    List_t edges = List_getFirst(v->edges);

    while (edges) {
        E e = (E) edges->data;
        V to = e->to;

        List_insertLast(result, to->data);
        edges = edges->next;
    }
    return result;
}

List_t Graph_predessors(T g, Poly_t k) {
    Set_t set = Set_new(g->equals);
    V v = searchVertex(g, k);
    List_t vs = List_getFirst(g->vs);

    while (vs) {
        V which = (V) vs->data;
        List_t edges = List_getFirst(which->edges);
        while (edges) {
            E e = (E) edges->data;
            V to = e->to;
            if (Vertex_equals(to, v))
                Set_insert(set, which->data);

            edges = edges->next;
        }
        vs = vs->next;
    }
    return Set_toList(set);
}


#undef T
#undef V
#undef E

#undef dprintf
