#include "peep-hole.h"

static List_t allStms = 0;

static void emitNode(List_t p) {
    List_appendNode(allStms, p);
}

static List_t getBeforeClear(void) {
    List_t t = allStms;
    allStms = 0;
    return t;
}

/* Match these patterns:
 *   1. store stack[i], r
 *      load  r, stack[i] (<==== eliminate)
 * Must be careful to stay in basic block.
 */
static void Trans_stms(List_t stms) {
    List_t first = List_getFirst(stms);
    while (first) {
        List_t p, second, q;
        X86_Stm_t s1, s2;

        p = first;
        second = p->next;
        if (!second) {
            emitNode(first);
            break;
        }
        q = second;
        s1 = (X86_Stm_t) p->data;
        s2 = (X86_Stm_t) q->data;
        if (s1->kind == X86_STM_STORE && s2->kind == X86_STM_LOAD) {
            X86_Operand_t storeDest = s1->u.store.dest,
                          loadSrc = s2->u.load.src;
            X86_Register_t storeSrc = s1->u.store.src,
                           loadDest = s2->u.load.dest;
            if (X86_Register_equals(storeSrc, loadDest) && X86_Operand_sameStackSlot(storeDest,
                                                                                     loadSrc)) {
                first = second->next;
                emitNode(p);
                continue;
            }
            first = q;
            emitNode(p);
            continue;
        }
        first = q;
        emitNode(p);
    }
}

static X86_Fun_t Trans_func(X86_Fun_t f) {
    List_t stms;
    allStms = List_new();
    Trans_stms(f->stms);
    stms = getBeforeClear();
    return X86_Fun_new(f->type,
                       f->name,
                       f->args,
                       f->decs,
                       stms,
                       f->retId,
                       f->entry,
                       f->exitt);
}


static X86_Prog_t PeepHole_shrinkTraced(X86_Prog_t p) {
    List_t funcs = List_map(p->funcs,
                            (Poly_tyId) Trans_func);
    return X86_Prog_new(p->strings,
                        p->masks,
                        funcs);
}

X86_Prog_t PeepHole_shrink(X86_Prog_t p) {
    return PeepHole_shrinkTraced(p);
}

#undef List_t
