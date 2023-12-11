#include "const-fold.h"
#include "../control/log.h"
#include "../lib/list.h"
#include "../lib/trace.h"

// this module has been incoporated into the const
// propogation pass and thus obsolete.


// whether or not constant-folded in one pass.
static int flag = 0;
int Changed_constFold = 0;

static void mark() {
    flag = 1;
    Changed_constFold = 1;
}

// Id_t -> Ssa_Operand_t
// map id to a constant, return 0 for none.
static Property_t constProp = 0;

//////////////////////////////////////////////////
// analysis

static void analyzeStm(Ssa_Stm_t s) {
    switch (s->kind) {
        case SSA_STM_BOP: {
            Id_t dest = s->u.bop.dest;
            Ssa_Operand_t left = s->u.bop.left;
            Ssa_Operand_t right = s->u.bop.right;

            //
            if (left->kind == SSA_OP_INT && right->kind == SSA_OP_INT) {
                mark();
                Log_str("found one const-fold candidate: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);
                long r = Operator_binary(left->u.intlit, s->u.bop.op, right->u.intlit);
                Property_set(constProp, dest, Ssa_Operand_new_int(r));
            }
            return;
        }
        case SSA_STM_UOP: {
            Id_t dest = s->u.uop.dest;
            Ssa_Operand_t src = s->u.uop.src;

            if (src->kind == SSA_OP_INT) {
                mark();
                Log_str("found one const-fold candidate: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);
                long r = Operator_unary(s->u.uop.op, src->u.intlit);
                Property_set(constProp, dest, Ssa_Operand_new_int(r));
            }
            return;
        }
        default:
            return;
    }
    return;
}

static void analyzeBlock(Ssa_Block_t b) {
    List_foreach(b->stms, (Poly_tyVoid) analyzeStm);
}

static void analyzeFun(Ssa_Fun_t f) {
    List_foreach(f->blocks, (Poly_tyVoid) analyzeBlock);
}

static void analyze(Ssa_Prog_t p) {

    List_foreach(p->funcs, (Poly_tyVoid) analyzeFun);
}

////////////////////////////////////////////////////////
// rewriting

// zero for non-const.
static Ssa_Operand_t rewriteOne(Id_t id) {
    return Property_get(constProp, id);
}

// don't eliminate the const assignment itself, for it
// would be a pretty easy job for dead code elimination
// to perform.
static List_t rewriteStmEach(List_t result, Ssa_Stm_t stm) {
    Ssa_Stm_t new = Ssa_Stm_renameUse2Op(stm, rewriteOne);
    if (new)
        List_insertLast(result, new);
    return result;
}

static Ssa_Block_t rewriteBlock(Ssa_Block_t b) {
    List_t newStms = List_new();
    Ssa_Transfer_t newTrans;

    List_foldl(b->stms, newStms, (Poly_tyFold) rewriteStmEach);
    newTrans = Ssa_Transfer_renameUse2Op(b->transfer, rewriteOne);
    return Ssa_Block_new(b->label, newStms, newTrans);
}

static Ssa_Fun_t rewriteFun(Ssa_Fun_t f) {
    List_t newBlocks;

    newBlocks = List_map(f->blocks, (Poly_tyId) rewriteBlock);
    return Ssa_Fun_new(f->type, f->name, f->args, f->decs, newBlocks, f->retId, f->entry, f->exitt);
}

static Ssa_Prog_t rewriteProg(Ssa_Prog_t p) {
    List_t newfuncs;

    newfuncs = List_map(p->funcs, (Poly_tyId) rewriteFun);
    return Ssa_Prog_new(p->classes, newfuncs);
}

////////////////////////////////////////////////
// program
static Ssa_Prog_t Ssa_constFoldTraced2(Ssa_Prog_t p) {
    constProp = Property_new((Poly_tyPlist) Id_plist);

    Log_str("analyzing starting:");
    analyze(p);
    Log_str("analyzing finished:");


    Log_str("propogation starting:");
    if (flag)
        p = rewriteProg(p);

    Log_str("propogation finished:");

    Property_clear(constProp);
    return p;
}

static Ssa_Prog_t Ssa_constFoldTraced(Ssa_Prog_t p) {
    do {
        flag = 0;
        p = Ssa_constFoldTraced2(p);
    } while (flag);
    return p;
}

static void printArg(Ssa_Prog_t p) {
    File_t file = File_open("constFold.arg", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

static void printResult(Ssa_Prog_t p) {
    File_t file = File_open("constFold.result", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

Ssa_Prog_t Ssa_constFold(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Log_POS();

    Trace_TRACE("Ssa_constFold", Ssa_constFoldTraced, (p), printArg, r, printResult);
    return r;
}
