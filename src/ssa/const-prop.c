#include "const-prop.h"
#include "../control/log.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/trace.h"

// This module performs const propagation and const
// folding. Here are good reasons to perform both
// of them together, rather than separately, consider
// such kind of code:
//   x1 = c;
//   x2 = x1 + c;
//   x3 = x2 + c;
// In summary, all analysis that help each other should
// be done simultaneously, and the rewriting should be
// lazy.

// can we found more constants?
static int foundMoreConst = 0;
static int shouldRewrite = 0;

static void mark(void) {
    foundMoreConst = 1;
    shouldRewrite = 1;
}

static int hasBeenMarked(void) {
    return 1 == foundMoreConst;
}

static void markClear(void) {
    foundMoreConst = 0;
    // don't clear the "shouldRewrite" here.
}

// Id_t -> Ssa_Operand_t
// map id to a constant, return 0 for none.
static Property_t constProp = 0;

//////////////////////////////////////////////////
// analysis

static void analyzeStm(Ssa_Stm_t s) {
    switch (s->kind) {
        case SSA_STM_MOVE: {
            Id_t dest = s->u.move.dest;
            Ssa_Operand_t src = s->u.move.src;

            //
            if (Property_get(constProp, dest))
                return;

            // the operand is a constant
            if (Ssa_Operand_isConst(src)) {
                mark();
                Log_str("found one const: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                Property_set(constProp, dest, src);
            }
            // the operand is not a constant, but a variable
            // which has been marked as a constant.
            else {
                Ssa_Operand_t v;
                Id_t right;

                if (src->kind != SSA_OP_ID)
                    Error_impossible();

                right = src->u.id;
                v = Property_get(constProp, right);
                // has been marked.
                if (v) {
                    mark();
                    Log_str("found a variable which is "
                            "marked const:");
                    Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                    Property_set(constProp, dest, v);
                }
            }
            return;
        }
            // dest <- left bop right
        case SSA_STM_BOP: {
            Id_t dest = s->u.bop.dest;
            Ssa_Operand_t left = s->u.bop.left;
            Ssa_Operand_t right = s->u.bop.right;
            //            int li, ri;

            if (Property_get(constProp, dest))
                return;

            // two (integer) constants
            if (left->kind == SSA_OP_INT && right->kind == SSA_OP_INT) {
                mark();
                Log_str("found (const, const) in binary op: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                long r = Operator_binary(left->u.intlit, s->u.bop.op, right->u.intlit);

                Property_set(constProp, dest, Ssa_Operand_new_int(r));
            }
            // an integer and an id
            else if (left->kind == SSA_OP_INT && right->kind == SSA_OP_ID) {
                Ssa_Operand_t rv = Property_get(constProp, right->u.id);

                if (rv && (rv->kind == SSA_OP_INT)) {
                    long r = Operator_binary(left->u.intlit, s->u.bop.op, rv->u.intlit);
                    mark();
                    Log_str("found (const, idconst) in binary op:");
                    Log_fun(s, (Poly_tyLog) Ssa_Stm_print);
                    Property_set(constProp, dest, Ssa_Operand_new_int(r));
                }
            }
            // and id and an integer
            else if (left->kind == SSA_OP_ID && right->kind == SSA_OP_INT) {
                Ssa_Operand_t lv = Property_get(constProp, left->u.id);

                if (lv && (lv->kind == SSA_OP_INT)) {
                    long r = Operator_binary(lv->u.intlit, s->u.bop.op, right->u.intlit);
                    mark();
                    Log_str("found (idconst, const) in binary op:");
                    Log_fun(s, (Poly_tyLog) Ssa_Stm_print);
                    Property_set(constProp, dest, Ssa_Operand_new_int(r));
                }
            }
            // two ids
            else if (left->kind == SSA_OP_ID && right->kind == SSA_OP_ID) {
                Ssa_Operand_t lv = Property_get(constProp, left->u.id);
                Ssa_Operand_t rv = Property_get(constProp, right->u.id);

                if (lv && (lv->kind == SSA_OP_INT) && rv && (rv->kind == SSA_OP_INT)) {
                    long r = Operator_binary(lv->u.intlit, s->u.bop.op, rv->u.intlit);
                    mark();
                    Log_str("found (idconst, idconst) in binary op:");
                    Log_fun(s, (Poly_tyLog) Ssa_Stm_print);
                    Property_set(constProp, dest, Ssa_Operand_new_int(r));
                }
            } else
                ;
            return;
        }
            // dest <- uop src
        case SSA_STM_UOP: {
            Id_t dest = s->u.uop.dest;
            Ssa_Operand_t src = s->u.uop.src;

            if (Property_get(constProp, dest))
                return;

            // integer
            if (src->kind == SSA_OP_INT) {
                mark();
                Log_str("found (const) in uop: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                long r = Operator_unary(s->u.uop.op, src->u.intlit);
                Property_set(constProp, dest, Ssa_Operand_new_int(r));
            }
            // id and marked
            else if (src->kind == SSA_OP_ID) {
                Ssa_Operand_t v = Property_get(constProp, src->u.id);
                if (v && (v->kind == SSA_OP_INT)) {
                    long r = Operator_unary(s->u.uop.op, v->u.intlit);

                    mark();
                    Log_str("found (idconst) in uop: ");
                    Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                    Property_set(constProp, dest, Ssa_Operand_new_int(r));
                } else
                    ;
            } else
                ;
            return;
        }
        case SSA_STM_PHI: {
            Id_t dest = s->u.phi.dest;
            List_t args = s->u.phi.args;

            if (Property_get(constProp, dest))
                return;

            if (Ssa_Stm_PhiArg_isSameConst(args)) {
                Ssa_Stm_PhiArg_t arg = args->next->data;

                mark();
                Log_str("found one const phi: ");
                Log_fun(s, (Poly_tyLog) Ssa_Stm_print);

                Property_set(constProp, dest, arg->arg);
            }
            return;
        }
        default: {
            return;
        }
    }
    //    return;
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
static Ssa_Prog_t Ssa_constPropTraced(Ssa_Prog_t p) {
    constProp = Property_new((Poly_tyPlist) Id_plist);

    Log_str("analyzing starting:");

    do {
        markClear();
        analyze(p);
    } while (hasBeenMarked());

    Log_str("analyzing finished:");

    if (shouldRewrite) {
        Log_str("rewriting starting:");
        p = rewriteProg(p);
        Log_str("rewriting finished:");
    }

    Property_clear(constProp);
    shouldRewrite = 0;
    return p;
}

static void printArg(Ssa_Prog_t p) {
    File_t file = File_open("constProp.arg", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

static void printResult(Ssa_Prog_t p) {
    File_t file = File_open("constProp.result", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

Ssa_Prog_t Ssa_constProp(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Log_POS();

    Trace_TRACE("Ssa_constProp", Ssa_constPropTraced, (p), printArg, r, printResult);
    return r;
}
