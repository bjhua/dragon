#include "dead-code.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/trace.h"
#include <assert.h>

/* This module has the following jobs:
 *   1. delete dead statements, where the dead statements
 *      are those like s1 and s2:
 *        return e;
 *        s1;
 *        s2;
 *   2. delete singleton variable-based expression-stm,
 *      as in these:
 *        x;
 *        y;
 */
static List_t Elab_stms(List_t);

static Hil_Stm_t Elab_stm(Hil_Stm_t s) {
    assert(s);
    switch (s->kind) {
        case HIL_STM_ASSIGN:
            return s;
        case HIL_STM_EXP: {
            Hil_Exp_t e = s->u.exp;
            if (e->kind == HIL_EXP_LVAL) {
                Hil_Lval_t l = e->u.lval;
                if (l->kind == HIL_LVAL_VAR)
                    return 0;
            }
            return s;
        }
        case HIL_STM_IF:
            return Hil_Stm_new_if(s->u.iff.cond,
                                  Elab_stms(s->u.iff.then),
                                  Elab_stms(s->u.iff.elsee));
        case HIL_STM_DO:
            return Hil_Stm_new_do(s->u.doo.cond,
                                  Elab_stms(s->u.doo.body),
                                  s->u.doo.entryLabel,
                                  s->u.doo.exitLabel,
                                  s->u.doo.padding);
        case HIL_STM_JUMP:
            return s;
        case HIL_STM_THROW: {
            return s;
        }
        case HIL_STM_TRYCATCH: {
            return s;
        }
        case HIL_STM_RETURN:
            Error_impossible();
            return 0;
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

static List_t Elab_stms(List_t stms) {
    List_t p, result = List_new();

    assert(stms);
    p = List_getFirst(stms);
    while (p) {
        Hil_Stm_t s = (Hil_Stm_t) p->data, newStm;

        if (s->kind == HIL_STM_RETURN) {
            List_insertLast(result, s);
            break;
        }
        newStm = Elab_stm(s);
        if (newStm)
            List_insertLast(result, newStm);
        p = p->next;
    }
    return result;
}

static Hil_Fun_t Elab_funEach(Hil_Fun_t f) {
    assert(f);
    return Hil_Fun_new(f->type,
                       f->name,
                       f->args,
                       f->decs,
                       Elab_stms(f->stms));
}

static List_t Elab_funcs(List_t fs) {
    assert(fs);
    return List_map(fs, (Poly_tyId) Elab_funEach);
}

static Hil_Prog_t Hil_deadCodeTraced(Hil_Prog_t p) {
    assert(p);
    return Hil_Prog_new(p->classes, Elab_funcs(p->funcs));
}

static void Trace_arg(Hil_Prog_t p) {
    assert(p);
    Hil_Prog_print(stdout, p);
}

Hil_Prog_t Hil_deadCode(Hil_Prog_t p) {
    Hil_Prog_t r;

    Trace_TRACE("Hil_deadCode",
                Hil_deadCodeTraced,
                (p),
                Trace_arg,
                r,
                Trace_arg);
    return r;
}
