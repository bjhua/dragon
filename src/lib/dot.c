#include "error.h"
#include "list.h"
#include "triple.h"
#include "file.h"
#include "mem.h"
#include "assert.h"
#include "system.h"
#include "dot.h"

#define T Dot_t

struct T {
    Poly_tyPrint printer;
    // List<Triple<P, P, P>>
    List_t list;
};

T Dot_new(Poly_tyPrint printer) {
    T d;

    Mem_NEW(d);
    d->printer = printer;
    d->list = List_new();
    return d;
}

void Dot_insert(T d, Poly_t from, Poly_t to, Poly_t info) {
    Triple_t t;

    Assert_ASSERT(d);
    /* newInfo = info? (String_concat ("[label = \"" */
    /*                                 , info */
    /*                                 , "\"]" */
    /*                                 , 0)): ""; */
    t = Triple_new(from, to, info);
    List_insertLast(d->list, t);
    return;
}

void Dot_toJpg(T d, String_t fname) {
    File_t fp;
    Poly_tyPrint printer;
    String_t dotfname = String_concat(fname, ".dot", 0);
    String_t jpgfname = String_concat(fname, ".dot", ".jpg", 0);

    Assert_ASSERT(d);
    printer = d->printer;

    //printf ("file name = %s\n", dotfname);

    fp = File_open(dotfname, "w+");
    File_write(fp, "digraph g{\n");
    File_write(fp, "\tsize = \"10, 10\";\n");
    File_write
            (fp, "\tnode [color=lightblue2, style=filled];\n");

    File_write(fp, "\n\n");

    {
        List_t p = List_getFirst(d->list);
        while (p) {
            Triple_t tr = (Triple_t) p->data;
            Poly_t third = Triple_third(tr);

            File_write(fp, "\"");
            printer(fp, Triple_first(tr));
            File_write(fp, "\"");
            File_write(fp, "->");
            File_write(fp, "\"");
            printer(fp, Triple_second(tr));
            File_write(fp, "\"");
            if (third) {
                File_write(fp, "[label = \"");
                printer(fp, third);
                File_write(fp, "\"]");
            }
            File_write(fp, ";\n");
            p = p->next;
        }
    }

    File_write(fp, "}\n\n");
    fclose(fp);
    {
        String_t cmd;

        cmd = String_concat("dot -Tjpg -o ", jpgfname, " ", dotfname, 0);
        //   printf ("run command: %s\n", cmd);
        System_run(cmd);
        // should modify this...
        if (0) {
            cmd = String_concat("rm ", dotfname, 0);
            System_run(cmd);
        }
    }
    return;
}
