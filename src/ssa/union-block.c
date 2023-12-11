#include "union-block.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include <assert.h>

// Label_t -> int
static Property_t numPreds = 0;
// Label_t -> {0|1}:
// whether or not this label should be unioned in
static Property_t victim = 0;

static Ssa_Fun_t fun = 0;
static Label_t funentry = 0;
static Label_t funexitt = 0;
static List_t globalBlocks = 0;

static void emit(Ssa_Block_t b) {
    List_insertLast(globalBlocks, b);
}

static int unioned = 0;

/////////////////////////////////////////////////////
// analyze

static void analyzeOneBlock(Ssa_Block_t b) {
    assert(b);
    assert(b->stms);
    assert(b->transfer);

    switch (b->transfer->kind) {
        case SSA_TRANS_IF: {
            Label_t tt = b->transfer->u.iff.truee;
            Label_t tf = b->transfer->u.iff.falsee;
            long tn = (long) Property_get(numPreds, tt);
            long fn = (long) Property_get(numPreds, tf);

            Property_set(numPreds, tt, (Poly_t) ++tn);
            Property_set(numPreds, tf, (Poly_t) ++fn);
            return;
        }
        case SSA_TRANS_JUMP: {
            Label_t l = b->transfer->u.jump;
            long ln = (long) Property_get(numPreds, l);

            Property_set(numPreds, l, (Poly_t) ++ln);
            return;
        }
        case SSA_TRANS_RETURN: {
            return;
        }
        case SSA_TRANS_THROW:
            return;
        case SSA_TRANS_CALL:
            return;
        default:
            Error_impossible();
            return;
    }
    Error_impossible();
    return;
}

///////////////////////////////////////////////////
// mark which block can be union into which
static void markVictim(Ssa_Block_t b) {
    Label_t label;

    assert(b);

    label = b->label;
    switch (b->transfer->kind) {
        case SSA_TRANS_IF: {
            // never union blocks into an "if" node
            return;
        }
        case SSA_TRANS_JUMP: {
            long n = (long) Property_get(numPreds, b->transfer->u.jump);
            if (n == 1 && !Label_equals(b->transfer->u.jump, label) && !Label_equals(b->transfer->u.jump, funentry)) {
                Property_set(victim, b->transfer->u.jump, (Poly_t) 1);
            } else
                ;
            return;
        }
        case SSA_TRANS_RETURN:
            return;
        case SSA_TRANS_THROW:
            return;
        case SSA_TRANS_CALL:
            return;
        default:
            Error_impossible();
            return;
    }
    Error_impossible();
    return;
}

//////////////////////////////////////////////////////
// store result in "globalBlocks"
static void transOne(Ssa_Block_t b) {
    assert(b);

    long v = (long) Property_get(victim, b->label);
    if (v)
        return;

    switch (b->transfer->kind) {
        case SSA_TRANS_IF: {
            emit(b);
            return;
        }
        case SSA_TRANS_JUMP: {
            Label_t jump = b->transfer->u.jump;
            Label_t label = b->label;
            long vj = (long) Property_get(victim, jump);
            Ssa_Block_t newb;

            if (vj) {
                Ssa_Block_t bj = Ssa_Fun_searchLabel(fun, jump);

                unioned = 1;

                if (!bj) {
                    Error_impossible();
                    return;
                }
                newb = Ssa_Block_new(b->label, List_concat(b->stms, bj->stms), bj->transfer);
                emit(newb);
                // maintain exit label
                if (Label_equals(funexitt, jump))
                    funexitt = label;
            } else
                emit(b);

            return;
        }
        case SSA_TRANS_RETURN: {
            emit(b);
            return;
        }
        case SSA_TRANS_THROW: {
            emit(b);
            return;
        }
        case SSA_TRANS_CALL:
            emit(b);
            return;
        default:
            Error_impossible();
            return;
    }
    Error_impossible();
    return;
}

//////////////////////////////////////////////////////
// functions
static Ssa_Fun_t transFunEach(Ssa_Fun_t f) {
    //    Ssa_Block_t eb;// entry block
    List_t newBlocks;
    //    Graph_t g;
    //    Label_t newExitt;

    assert(f);

    fun = f;
    funentry = f->entry;
    funexitt = f->exitt;
    globalBlocks = List_new();
    newBlocks = f->blocks;

    // calculate num of preds and succs (really need succs?)
    List_foreach(f->blocks, (Poly_tyVoid) analyzeOneBlock);

    // mark which block can be unioned (the entry block
    // will never be unioned)
    List_foreach(f->blocks, (Poly_tyVoid) markVictim);

    // transform blocks
    do {
        unioned = 0;
        List_foreach(newBlocks, (Poly_tyVoid) transOne);
        newBlocks = globalBlocks;
        globalBlocks = List_new();
    } while (unioned);

    return Ssa_Fun_new(f->type, f->name, f->args, f->decs, newBlocks, f->retId, f->entry, funexitt);
}


//////////////////////////////////////////////////////
// programs
static Ssa_Prog_t Ssa_unionBlockTraced(Ssa_Prog_t p) {
    List_t newFuncs;

    assert(p);

    // init the property
    numPreds = Property_new((Poly_tyPlist) Label_plist);
    victim = Property_new((Poly_tyPlist) Label_plist);


    newFuncs = List_map(p->funcs, (Poly_tyId) transFunEach);

    // clear the property
    Property_clear(numPreds);
    Property_clear(victim);

    return Ssa_Prog_new(p->classes, newFuncs);
}

/////////////////////////////////////////////////////
// main functions
static void printArg(Ssa_Prog_t p) {
    Ssa_Prog_toDot(p, "beforeSsaUnionBlock");
    Ssa_Prog_print(stdout, p);
    return;
}

static void printResult(Ssa_Prog_t p) {
    Ssa_Prog_toDot(p, "afterSsaUnionBlock");
    Ssa_Prog_print(stdout, p);
    return;
}

Ssa_Prog_t Ssa_unionBlock(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Trace_TRACE("Ssa_unionBlock", Ssa_unionBlockTraced, (p), printArg, r, printResult);
    return r;
}
