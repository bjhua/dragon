#include <stdio.h>
#include "test.h"
#include "error.h"
#include "trace.h"
#include "property-list.h"
#include "property.h"
#include "../atoms/id.h"
#include "poly.h"
#include "box.h"
#include "set.h"
#include "graph.h"


void printer(File_t f, String_t s) {
    fprintf(f, "%s", s);
}

int Lib_testTraced(int junk) {
    // test set
    {
        Set_t set = Set_new((Poly_tyEquals) String_equals);
        Set_t set2 = Set_new((Poly_tyEquals) String_equals);

        Set_insert(set, "x");
        Set_insert(set, "y");
        Set_insert(set, "z");
        Set_insert(set, "x");
        Set_insert(set, "y");
        Set_insert(set, "z");
        Set_foreach(set, (Poly_tyVoid) String_print);
        printf("\n");

        Set_insert(set2, "x");
        Set_insert(set2, "y");
        Set_insert(set2, "x");
        Set_insert(set2, "y");
        Set_insert(set2, "w");
        Set_foreach(set2, (Poly_tyVoid) String_print);

        printf("\nequals = %d\n", Set_equals(set, set2));

        Set_foreach(Set_intersection(set, set2), (Poly_tyVoid) String_print);

        printf(" \\/ = ");
        Set_foreach(Set_union(set, set2), (Poly_tyVoid) String_print);
        printf("\ntesting set finished\n");
    }

    // to test graphs, this is the graph from Figure 7-4
    // of Muchnick.
    {
        /*
                    a
                   /  \
                  b    c
                   \  /
                    d
        */
        Graph_t g
                = Graph_newWithName((Poly_tyEquals) String_equals, "test");
        Tree_t tree;

        Graph_insertVertex(g, "entry");
        Graph_insertVertex(g, "B1");
        Graph_insertVertex(g, "B2");
        Graph_insertVertex(g, "B3");
        Graph_insertVertex(g, "B4");
        Graph_insertVertex(g, "B5");
        Graph_insertVertex(g, "B6");
        Graph_insertVertex(g, "exit");
        Graph_insertEdge(g, "entry", "B1");
        Graph_insertEdge(g, "B1", "B2");
        Graph_insertEdge(g, "B2", "exit");
        Graph_insertEdge(g, "B1", "B3");
        Graph_insertEdge(g, "B3", "B4");
        Graph_insertEdge(g, "B4", "B5");
        Graph_insertEdge(g, "B4", "B6");
        Graph_insertEdge(g, "B5", "exit");
        Graph_insertEdge(g, "B6", "B4");

        Graph_toJpg(g, (Poly_tyPrint) printer);

        //
        tree = Graph_domTree(g, "entry");

        Tree_toJpgWithName(tree, (Poly_tyPrint) printer, "dom");

        printf("\ntesting graph finished\n");
    }
    {
        printf("\ntesting hash-set starting\n");

        printf("\ntesting hash-set finished\n");
    }
    return 0;
}

void Lib_arg(int r) {
    return;
}

int Lib_test() {
    int r;

    Trace_TRACE("Lib_test", Lib_testTraced, (0), Lib_arg, r, Lib_arg);
    return r;
}

