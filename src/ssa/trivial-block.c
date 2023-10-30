#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/property.h"
#include "../lib/trace.h"
#include "dead-block.h"
#include "trivial-block.h"

// on <Label_t>
static Property_t jumpto = 0;

/////////////////////////////////////////////////////
// analyze

static void analyzeOneBlock(Ssa_Block_t b) {
    Assert_ASSERT(b);
    Assert_ASSERT(b->stms);
    Assert_ASSERT(b->transfer);

    if (List_isEmpty(b->stms)) {
        if (b->transfer->kind == SSA_TRANS_JUMP)
            Property_set(jumpto, b->label, b->transfer->u.jump);
        else;
    } else;
    return;
}

//////////////////////////////////////////////////////
static Ssa_Block_t transOne(Ssa_Block_t b) {
    Assert_ASSERT(b);

    switch (b->transfer->kind) {
        case SSA_TRANS_IF: {
            Label_t newt = 0, newf = 0;

            newt = Property_get(jumpto, b->transfer->u.iff.truee);
            newf =
                    Property_get(jumpto, b->transfer->u.iff.falsee);

            return Ssa_Block_new(b->label, b->stms, Ssa_Transfer_renameLabels_if
                    (b->transfer, newt, newf));
        }
        case SSA_TRANS_JUMP: {
            Label_t newj = 0;

            newj = Property_get(jumpto, b->transfer->u.jump);
            return Ssa_Block_new(b->label, b->stms, Ssa_Transfer_renameLabels_jump
                    (b->transfer, newj));
        }
        case SSA_TRANS_RETURN:
            return b;
        case SSA_TRANS_THROW:
            return b;
        case SSA_TRANS_CALL:
            return b;
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

//////////////////////////////////////////////////////
// functions
static Ssa_Fun_t transFunEach(Ssa_Fun_t f) {
    Ssa_Block_t eb;    // entry block
    List_t newBlocks;
    Graph_t g;
    Label_t newExitt;

    Assert_ASSERT(f);

    // analyze
    List_foreach(f->blocks, (Poly_tyVoid) analyzeOneBlock);

    // transform blocks
    newBlocks = List_map(f->blocks, (Poly_tyId) transOne);

    // eliminate dead blocks?

    // exit entry may be modified (should we maintain this?)
    newExitt = Property_get(jumpto, f->exitt);
    if (!newExitt)
        newExitt = f->exitt;

    return Ssa_Fun_new(f->type, f->name, f->args, f->decs, newBlocks, f->retId, f->entry, newExitt);
}


//////////////////////////////////////////////////////
// programs
static Ssa_Prog_t Ssa_trivialBlockTraced(Ssa_Prog_t p) {
    List_t newFuncs;

    Assert_ASSERT(p);

    // init the property
    jumpto = Property_new((Poly_tyPlist) Label_plist);

    newFuncs = List_map(p->funcs, (Poly_tyId) transFunEach);

    // clear property
    Property_clear(jumpto);

    return Ssa_Prog_new(p->classes, newFuncs);
}

/////////////////////////////////////////////////////
// main functions
static void printArg(Ssa_Prog_t p) {
    Ssa_Prog_toDot(p, "beforeSsaTrivialBlock");
    return;
}

static void printResult(Ssa_Prog_t p) {
    Ssa_Prog_toDot(p, "afterSsaTrivialBlock");
    return;
}

static Ssa_Prog_t Ssa_trivialBlock2(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Trace_TRACE("Ssa_trivialBlock", Ssa_trivialBlockTraced, (p), printArg, r, printResult);
    return r;
}

Ssa_Prog_t Ssa_trivialBlock(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    r = Ssa_deadBlock(Ssa_trivialBlock2(p));
    return r;
}
