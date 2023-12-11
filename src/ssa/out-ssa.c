#include "out-ssa.h"
#include "../control/log.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/property.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"
#include "../lib/unused.h"
#include <assert.h>

/////////////////////////////////////////////////////
// properties:
// Ssa_Block_t -> List<Stm_t>
// given a block, return statements that should append to
// its head
static Property_t headProp = 0;

static List_t headPropInitFun(Ssa_Block_t b) {
    UNUSED(b);
    return List_new();
}

// Ssa_Block_t -> List<Stm_t>
// given a block, return statements that should append to
// its tail
static Property_t tailProp = 0;

static List_t tailPropInitFun(Ssa_Block_t b) {
    UNUSED(b);
    return List_new();
}

// Id_t -> Tuple<Id_t, Id_t>
// given an id, return the fresh "dest" and "arg"
// ids generated for it
static Property_t nameProp = 0;


//////////////////////////////////////////////////////
// analysis

static void analyzeOne(Ssa_Block_t b) {
    List_t stms = List_getFirst(b->stms);
    while (stms) {
        Ssa_Stm_t s = stms->data;

        if (s->kind != SSA_STM_PHI)
            return;

        {
            Id_t dest = s->u.phi.dest;
            List_t args = List_getFirst(s->u.phi.args);
            Id_t newa = Id_newNoName();
            Id_t newdest = Id_newNoName();
            List_t oldhead;

            Property_set(nameProp, dest, Tuple_new(newdest, newa));

            while (args) {
                Ssa_Stm_PhiArg_t a = args->data;
                Ssa_Operand_t opr = a->arg;
                Ssa_Block_t pred = a->pred;

                List_t oldtail = Property_get(tailProp, pred);

                List_insertLast(oldtail, Ssa_Stm_new_move(newa, opr));
                List_insertLast(oldtail, Ssa_Stm_new_move(newdest, Ssa_Operand_new_id(newa)));
                args = args->next;
            }
            oldhead = Property_get(headProp, b);
            List_insertLast(oldhead, Ssa_Stm_new_move(dest, Ssa_Operand_new_id(newdest)));
        }

        stms = stms->next;
    }
    return;
}

static void analyze(List_t blocks) {
    List_foreach(blocks, (Poly_tyVoid) analyzeOne);
}

//////////////////////////////////////////////////////
// rewrite

static Ssa_Block_t rewriteOne(Ssa_Block_t b) {
    List_t stms, heads, tails;

    heads = Property_get(headProp, b);
    if (!heads)
        heads = List_new();
    tails = Property_get(tailProp, b);

    stms = List_getFirst(b->stms);
    while (stms) {
        Ssa_Stm_t s = stms->data;
        if (s->kind == SSA_STM_PHI) {
            Log_str("found ssa:");
            Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

            stms = stms->next;
            continue;
        }
        List_insertLast(heads, s);
        stms = stms->next;
    }
    if (tails)
        List_append(heads, tails);

    return Ssa_Block_new(b->label, heads, b->transfer);
}

static List_t rewrite(List_t blocks) {
    return List_map(blocks, (Poly_tyId) rewriteOne);
}

static List_t rewriteDecs(List_t args, List_t decs) {
    List_t result = List_new();

    args = List_getFirst(args);
    while (args) {
        Dec_t d = args->data;
        Dec_t new;
        Tuple_t t = Property_get(nameProp, d->id);
        if (!t) {
            args = args->next;
            continue;
        }
        new = Dec_new(d->ty, Tuple_first(t));
        List_insertLast(result, new);

        new = Dec_new(d->ty, Tuple_second(t));
        List_insertLast(result, new);
        args = args->next;
    }

    args = List_getFirst(decs);
    while (args) {
        Dec_t d = args->data;
        Dec_t new;
        Tuple_t t = Property_get(nameProp, d->id);

        List_insertLast(result, d);

        if (!t) {
            args = args->next;
            continue;
        }
        new = Dec_new(d->ty, Tuple_first(t));
        List_insertLast(result, new);

        new = Dec_new(d->ty, Tuple_second(t));
        List_insertLast(result, new);
        args = args->next;
    }
    return result;
}

//////////////////////////////////////////////////////
// functions
static Ssa_Fun_t transFunEach(Ssa_Fun_t f) {
    Ssa_Fun_t newf;
    List_t blocks, decs;

    assert(f);

    Log_str("analysis starting:");
    analyze(f->blocks);

    Log_str("analysis finished:");

    blocks = rewrite(f->blocks);

    decs = rewriteDecs(f->args, f->decs);

    newf = Ssa_Fun_new(f->type, f->name, f->args, decs, blocks, f->retId, f->entry, f->exitt);
    return newf;
}


//////////////////////////////////////////////////////
// programs
static Ssa_Prog_t Ssa_outSsaTraced(Ssa_Prog_t p) {
    List_t newFuncs;

    assert(p);
    headProp = Property_newInitFun((Poly_tyPlist) Ssa_Block_plist, (Poly_tyId) headPropInitFun);
    tailProp = Property_newInitFun((Poly_tyPlist) Ssa_Block_plist, (Poly_tyId) tailPropInitFun);
    nameProp = Property_new((Poly_tyPlist) Id_plist);

    newFuncs = List_map(p->funcs, (Poly_tyId) transFunEach);

    Property_clear(headProp);
    Property_clear(tailProp);
    Property_clear(nameProp);
    return Ssa_Prog_new(p->classes, newFuncs);
}

/////////////////////////////////////////////////////
// main functions
static void printArg(Ssa_Prog_t p) {
    //Ssa_Prog_toDot (p, "beforeOutSsa");
    // and also print it out
    {
        File_t file = File_open("outSsa.arg", "w+");
        Ssa_Prog_print(file, p);
        File_close(file);
    }
    return;
}

static void printResult(Ssa_Prog_t p) {
    //Ssa_Prog_toDot (p, "afterOutSsa");
    // and also print it out
    {
        File_t file = File_open("outSsa.result", "w+");
        Ssa_Prog_print(file, p);
        File_close(file);
    }
    return;
}

Ssa_Prog_t Ssa_outSsa(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Log_POS();

    Trace_TRACE("Ssa_outSsa", Ssa_outSsaTraced, (p), printArg, r, printResult);
    return r;
}
