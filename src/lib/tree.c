#include "tree.h"
#include "dot.h"
#include "error.h"
#include "list.h"
#include "mem.h"
#include "property-list.h"
#include "property.h"
#include <assert.h>
#include <stdio.h>

#define T Tree_t
#define V Vertex_t
#define E Edge_t

struct T {
    String_t name;
    Poly_tyEquals equals;
    List_t vs;
};

typedef struct V *V;
typedef struct E *E;

///////////////////////////////////////////////////////
// vertex
struct V {
    Poly_t data;
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
    assert(v);

    return v->plist;
}

//static int Vertex_equals(V v1, V v2) {
//    assert(v1);
//    assert(v2);
//
//    return v1 == v2;
//}

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

//static E Edge_fromData(Poly_t from, Poly_t to) {
//    E e;
//
//    Error_impossible();
//    return e;
//}


//static Plist_t Edge_plist(E e) {
//    assert(e);
//
//    return e->plist;
//}


/////////////////////////////////////////////////////
// tree
T Tree_new(Poly_tyEquals eq) {
    T g;

    Mem_NEW(g);
    g->name = "NONE";
    g->equals = eq;
    g->vs = List_new();
    return g;
}

T Tree_newWithName(Poly_tyEquals eq, String_t name) {
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

    assert(g);
    assert(data);

    p = List_getFirst(g->vs);
    equals = g->equals;
    while (p) {
        V v = (V) (p->data);

        if (equals(data, v->data))
            return v;
        p = p->next;
    }
    Error_error("vertex not found: tree.c\n");
    return 0;
}

//static E searchEdge(T g, Poly_t from, Poly_t to) {
//    V fv = searchVertex(g, from);
//    List_t edges = List_getFirst(fv->edges);
//
//    while (edges) {
//        E current = (E) (edges->data);
//        V currentTo = current->to;
//        if (g->equals(to, currentTo->data))
//            return current;
//        edges = edges->next;
//    }
//    Error_error("no this edge: searchEdge: graph.c\n");
//    return 0;
//}

/////////////////////////////////////////////////////
//
void Tree_insertVertex(T g, Poly_t x) {
    V v = Vertex_new(x);
    List_insertLast(g->vs, v);
    return;
}

void Tree_insertEdge(T g, Poly_t from, Poly_t to) {
    V fv = searchVertex(g, from);
    V tv = searchVertex(g, to);
    E e = Edge_new(fv, tv);
    List_insertLast(fv->edges, e);
    return;
}

void Tree_toJpgWithName(T g, Poly_tyPrint printer, String_t fname) {
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

void Tree_toJpg(T g, Poly_tyPrint printer) {
    Tree_toJpgWithName(g, printer, "test");
}

///////////////////////////////////////////
// depth-first search
static void Tree_dfsDoit(T g, V v, Poly_tyVoid f, Property_t visited) {
    List_t edges;

    f(v->data);
    Property_set(visited, v, (Poly_t) 1);
    edges = List_getFirst(v->edges);
    while (edges) {
        E e = (E) edges->data;
        V to = e->to;
        Poly_t visitedTo = Property_get(visited, to);
        if (!visitedTo) {
            Tree_dfsDoit(g, to, f, visited);
        } else
            ;
        edges = edges->next;
    }
    return;
}

void Tree_dfs(T g, Poly_t start, Poly_tyVoid f) {
    V sv;
    Property_t visited;

    assert(g);
    assert(start);
    assert(f);

    sv = searchVertex(g, start);
    visited = Property_new((Poly_tyPlist) Vertex_plist);
    if (sv)
        Tree_dfsDoit(g, sv, f, visited);

    // don't visit nodes unreachable from start
    // may add a flag to control this

    Property_clear(visited);
    return;
}

List_t Tree_children(T t, Poly_t v) {
    V sv;
    List_t result = List_new();

    assert(t);

    sv = searchVertex(t, v);
    {
        List_t edges = List_getFirst(sv->edges);
        while (edges) {
            E e = (E) edges->data;
            V to = e->to;
            Poly_t data = to->data;

            List_insertLast(result, data);
            edges = edges->next;
        }
    }
    return result;
}

Tree_t Tree_map(T t, Poly_tyEquals equals, Poly_t (*map)(Poly_t)) {
    Tree_t newt = Tree_new(equals);
    List_t vs = List_getFirst(t->vs);

    while (vs) {
        V v = (V) vs->data;

        Tree_insertVertex(newt, map(v->data));
        vs = vs->next;
    }
    vs = List_getFirst(t->vs);
    while (vs) {
        V v = (V) vs->data;
        List_t edges = List_getFirst(v->edges);
        while (edges) {
            E e = (E) edges->data;
            V from = e->from;
            V to = e->to;

            Tree_insertEdge(newt, map(from->data), map(to->data));
            edges = edges->next;
        }
        vs = vs->next;
    }
    return newt;
}


#undef T
#undef V
#undef E
