#include "lift-dec.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/trace.h"
#include <assert.h>

static List_t allDecs = 0;

static void genDecs(List_t l) {
    List_append(allDecs, l);
}

static Ast_Block_t Elab_block(Ast_Block_t);

static Ast_Stm_t Elab_stm(Ast_Stm_t s) {
    assert(s);
    switch (s->kind) {
        case AST_STM_EXP: {
            return s;
        }
        case AST_STM_IF: {
            return Ast_Stm_new_if(s->u.iff.cond, Elab_stm(s->u.iff.then), Elab_stm(s->u.iff.elsee), s->region);
        }
        case AST_STM_WHILE: {
            return Ast_Stm_new_while(s->u.whilee.cond, Elab_stm(s->u.whilee.body), s->region);
        }
        case AST_STM_DO: {
            return Ast_Stm_new_do(s->u.doo.cond, Elab_stm(s->u.doo.body), s->region);
        }
        case AST_STM_FOR: {
            return Ast_Stm_new_for(s->u.forr.header, s->u.forr.cond, s->u.forr.tail, Elab_stm(s->u.forr.body),
                                   s->region);
        }
        case AST_STM_BREAK:
            return s;
        case AST_STM_CONTINUE:
            return s;
        case AST_STM_THROW: {
            return s;
        }
        case AST_STM_TRYCATCH: {
            return Ast_Stm_new_tryCatch(Elab_stm(s->u.trycatch.tryy), Elab_stm(s->u.trycatch.catchh), s->region);
        }
        case AST_STM_RETURN: {
            return s;
        }
        case AST_STM_BLOCK:
            return Ast_Stm_new_block(Elab_block(s->u.block));
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

static Ast_Block_t Elab_block(Ast_Block_t b) {
    List_t stms;

    assert(b);
    genDecs(b->decs);
    stms = List_map(b->stms, (Poly_tyId) Elab_stm);
    return Ast_Block_new(List_new(), stms);
}

static Ast_Fun_t Elab_funForOne(Ast_Fun_t f) {
    Ast_Block_t b;

    assert(f);
    allDecs = List_new();
    b = Elab_block(f->block);
    b = Ast_Block_new(allDecs, b->stms);
    return Ast_Fun_new(f->type, f->name, f->args, b, f->region);
}

static List_t Elab_funs(List_t fs) {
    List_t list;

    assert(fs);
    list = List_map(fs,
                    (Poly_tyId) Elab_funForOne);
    return list;
}

static Ast_Prog_t Lift_dec_traced(Ast_Prog_t p) {
    List_t funcs;

    assert(p);
    funcs = Elab_funs(p->funcs);
    return Ast_Prog_new(p->classes, funcs);
}

static void Trace_arg(Ast_Prog_t p) {
    File_saveToFile("lift-dec.arg", (Poly_tyPrint) Ast_Prog_print, p);
    return;
}

static void Trace_result(Ast_Prog_t p) {
    File_saveToFile("lift-dec.result", (Poly_tyPrint) Ast_Prog_print, p);
    return;
}

Ast_Prog_t Lift_dec(Ast_Prog_t p) {
    Ast_Prog_t r;

    Trace_TRACE("Lift_dec", Lift_dec_traced, (p), Trace_arg, r, Trace_result);
    return r;
}
