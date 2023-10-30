#include <assert.h>
#include "../lib/assert.h"
#include "../lib/list.h"
#include "../lib/error.h"
#include "../lib/mem.h"
#include "../lib/property.h"
#include "../lib/trace.h"
#include "../atoms/atoms.h"
#include "../control/control.h"
#include "../control/log.h"
#include "trans-hil.h"

//////////////////////////////////////////////////////
// function information
struct FunInfo_t {
    Id_t retId;
    Label_t entryLabel;
    Label_t exitLabel;
};

static struct FunInfo_t fun = {0, 0, 0};

//////////////////////////////////////////////////////
// some dubugging utilities for control flow

#define S_IF_TRUE     ("IF_TRUE")
#define S_IF_FALSE    ("IF_FALSE")
#define S_IF_END      ("IF_END")

#define S_WHILE_START ("WHILE_START")
// label for break or continue
#define S_WHILE_BC    ("WHILE_BC")
#define S_WHILE_END   ("WHILE_END")

/////////////////////////////////////////////////////
// Some auxilary information on a label
// Id_t -> List_t
static Property_t fieldProp = 0;

static List_t fieldPropInitFun(Id_t id) {
    return List_new();
}

/////////////////////////////////////////////////////
// Some auxilary information on a label
// Label_t -> String_t
static Property_t labelInfoProp = 0;

static void LabelInfo_init() {
    if (Control_labelInfo)
        labelInfoProp
                = Property_new((Poly_tyPlist) Label_plist);
}

static void LabelInfo_clear() {
    if (Control_labelInfo)
        Property_clear(labelInfoProp);
}

static Label_t genLabel(String_t s) {
    Label_t fresh = Label_new();

    if (!Control_labelInfo)
        return fresh;

    Property_set(labelInfoProp, fresh, s ? s : "NO_INFO");
    return fresh;
}

static void printLabel(Label_t l) {
    Label_print(l);
    printf(": ");
    if (labelInfoProp) {
        String_t info = Property_get(labelInfoProp, l);
        printf("%s", info ? info : "NOT");
    }
    printf("\n");
    return;
}

//////////////////////////////////////////////////////
// handle cache

// Tanslation of a Hil.stm results a list of 
// type "Cache_Data_t".
typedef struct {
    enum {
        LABEL,
        STM,
        TRANS
    } kind;
    union {
        Label_t label;
        Ssa_Stm_t stm;
        Ssa_Transfer_t trans;
    } u;
} *Cache_Data_t;

// List<Cache_Data_t>
static List_t caches = 0;

static void emitLabel(Label_t s) {
    Cache_Data_t p;

    Mem_NEW(p);
    p->kind = LABEL;
    p->u.label = s;
    List_insertLast(caches, p);
}

static void emitStm(Ssa_Stm_t s) {
    Cache_Data_t p;

    Mem_NEW(p);
    p->kind = STM;
    p->u.stm = s;
    List_insertLast(caches, p);
}

static void emitTrans(Ssa_Transfer_t s) {
    Cache_Data_t p;

    Mem_NEW(p);
    p->kind = TRANS;
    p->u.trans = s;
    List_insertLast(caches, p);
}

static List_t getBeforeClear() {
    List_t t = caches;
    caches = List_new();
    return t;
}

static void Cache_log() {
    Assert_ASSERT(caches);

    List_t p = List_getFirst(caches);
    while (p) {
        Cache_Data_t data = (Cache_Data_t) p->data;

        switch (data->kind) {
            case LABEL:
                // may print more informations
                Log_str(Label_toString(data->u.label));
                if (labelInfoProp) {
                    String_t info = Property_get(labelInfoProp, data->u.label);
                    Log_str(info ? info : "NOT");
                }
                break;
            case STM:
                Log_fun(data->u.stm, (Poly_tyLog) Ssa_Stm_print);
                break;
            case TRANS:
                Log_fun(data->u.trans, (Poly_tyLog) Ssa_Transfer_print);
                break;
            default:
                Error_impossible ();
                return;
        }
        p = p->next;
    }
    return;
}


/////////////////////////////////////////////////////
// cache for declarations
static List_t allDecs = 0;

static Id_t genDec(Atype_t ty) {
    Id_t id;
    Dec_t newDec;

    Assert_ASSERT(ty);
    id = Id_newNoName();
    newDec = Dec_new(ty, id);
    List_insertLast(allDecs, newDec);
    return id;
}

static List_t getDecs() {
    List_t t = allDecs;
    allDecs = List_new();
    return t;
}


static Ssa_Operand_t Trans_exp(Hil_Exp_t);


////////////////////////////////////////////////////
// translation of left-value
struct Lval_Result_t {
    enum {
        ID,
        MEM
    } kind;
    union {
        Id_t id;
        Ssa_Mem_t mem;
    } u;
};

static Id_t Trans_lvalRight(Hil_Lval_t l) {
    Assert_ASSERT(l);
    switch (l->kind) {
        case HIL_LVAL_VAR:
            return l->u.var;
        case HIL_LVAL_DOT: {
            Hil_Lval_t left = l->u.dot.lval;
            Id_t id = Trans_lvalRight(left);
            Id_t fresh;
            Ssa_Mem_t mem = Ssa_Mem_new_class(id, l->u.dot.var);

            Assert_ASSERT(l->ty);
            fresh = genDec(l->ty);
            emitStm(Ssa_Stm_new_load(fresh, mem));
            return fresh;
        }
        case HIL_LVAL_ARRAY: {
            Hil_Lval_t left = l->u.array.lval;
            Id_t id = Trans_lvalRight(left);
            Ssa_Operand_t opd = Trans_exp(l->u.array.exp);
            Ssa_Mem_t mem = Ssa_Mem_new_array(id, opd);

            Id_t fresh = genDec(left->ty);
            emitStm(Ssa_Stm_new_load(fresh, mem));
            return fresh;
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

static struct Lval_Result_t Trans_lvalLeft(Hil_Lval_t l) {
    struct Lval_Result_t result;

    Assert_ASSERT(l);
    switch (l->kind) {
        case HIL_LVAL_VAR: {
            result.kind = ID;
            result.u.id = l->u.var;
            return result;
        }
        case HIL_LVAL_DOT: {
            Id_t left = Trans_lvalRight(l->u.dot.lval);
            Ssa_Mem_t mem = Ssa_Mem_new_class(left, l->u.dot.var);

            result.kind = MEM;
            result.u.mem = mem;
            return result;
        }
        case HIL_LVAL_ARRAY: {
            Id_t left = Trans_lvalRight(l->u.array.lval);
            Ssa_Operand_t opd
                    = Trans_exp(l->u.array.exp);
            Ssa_Mem_t mem
                    = Ssa_Mem_new_array(left, opd);

            result.kind = MEM;
            result.u.mem = mem;
            return result;
        }
        default:
            Error_impossible();
            return result;
    }
    Error_impossible();
    return result;
}

static Ssa_Operand_t Trans_bop(Hil_Exp_t left,
                               Operator_t op,
                               Hil_Exp_t right,
                               Atype_t ty) {
    switch (op) {
        case OP_AND: {
            Ssa_Operand_t newLeft;
            Ssa_Operand_t newRight;
            Id_t resultId = genDec(ty);
            Ssa_Operand_t result = Ssa_Operand_new_id(resultId);
            Label_t tl = Label_new(),
                    fl = Label_new(),
                    el = Label_new();

            newLeft = Trans_exp(left);
            emitTrans(Ssa_Transfer_new_if(newLeft,
                                          tl,
                                          fl));
            emitLabel(tl);
            newRight = Trans_exp(right);
            emitStm(Ssa_Stm_new_move(resultId, newRight));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(fl);
            emitStm(Ssa_Stm_new_move
                            (resultId,
                             Ssa_Operand_new_int(0)));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(el);
            return result;
        }
        case OP_OR: {
            Ssa_Operand_t newLeft;
            Ssa_Operand_t newRight;
            Id_t resultId = genDec(ty);
            Ssa_Operand_t result = Ssa_Operand_new_id(resultId);
            Label_t tl = Label_new(),
                    fl = Label_new(),
                    el = Label_new();

            newLeft = Trans_exp(left);
            emitTrans(Ssa_Transfer_new_if(newLeft,
                                          tl,
                                          fl));
            emitLabel(tl);
            emitStm(Ssa_Stm_new_move
                            (resultId,
                             Ssa_Operand_new_int(1)));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(fl);
            newRight = Trans_exp(right);
            emitStm(Ssa_Stm_new_move(resultId, newRight));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(el);
            return result;
        }
        default: {
            Ssa_Operand_t newLeft = Trans_exp(left);
            Ssa_Operand_t newRight = Trans_exp(right);
            Id_t resultId = genDec(ty);

            emitStm(Ssa_Stm_new_bop(resultId,
                                    newLeft,
                                    op,
                                    newRight));
            return Ssa_Operand_new_id(resultId);
        }
    }
    Error_impossible ();
    return 0;
}

static Ssa_Operand_t Trans_uop(Hil_Exp_t src,
                               Operator_t op,
                               Atype_t ty) {
    switch (op) {
        case OP_NOT: {
            Ssa_Operand_t newE = Trans_exp(src);
            Id_t resultId = genDec(ty);
            Ssa_Operand_t result = Ssa_Operand_new_id(resultId);
            Label_t tl = Label_new(),
                    fl = Label_new(),
                    el = Label_new();

            emitTrans(Ssa_Transfer_new_if(newE,
                                          tl,
                                          fl));
            emitLabel(tl);
            emitStm(Ssa_Stm_new_move(resultId,
                                     Ssa_Operand_new_int(0)));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(fl);
            emitStm(Ssa_Stm_new_move(resultId,
                                     Ssa_Operand_new_int(1)));
            emitTrans(Ssa_Transfer_new_jump(el));
            emitLabel(el);
            return result;
        }
        default: {
            Ssa_Operand_t newE = Trans_exp(src);
            Id_t resultId = genDec(ty);

            emitStm(Ssa_Stm_new_uop(resultId,
                                    op,
                                    newE));
            return Ssa_Operand_new_id(resultId);
        }
    }
    Error_impossible ();
    return 0;
}

static Ssa_Operand_t Trans_exp(Hil_Exp_t e) {
    Assert_ASSERT(e);
    switch (e->kind) {
        case HIL_EXP_BOP: {
            Ssa_Operand_t r = Trans_bop(e->u.bop.left,
                                        e->u.bop.op,
                                        e->u.bop.right,
                                        e->ty);
            return r;
        }
        case HIL_EXP_UOP: {
            return Trans_uop(e->u.unary.e,
                             e->u.unary.op,
                             e->ty);
        }
        case HIL_EXP_INTLIT: {
            return Ssa_Operand_new_int(e->u.intlit);
        }
        case HIL_EXP_STRINGLIT: {
            return Ssa_Operand_new_string(e->u.stringlit);
        }
        case HIL_EXP_NEW_CLASS: {
            Id_t className = e->u.newClass.name;
            List_t args = e->u.newClass.args;
            Id_t fresh;
            List_t fields = Property_get(fieldProp, className);
            List_t ops = List_new();

            fresh = genDec(Atype_new_class(className));

            emitStm(Ssa_Stm_new_newClass(fresh, className));
            args = List_getFirst(args);
            while (args) {
                Hil_Exp_t a = (Hil_Exp_t) args->data;
                Ssa_Operand_t o = Trans_exp(a);

                List_insertLast(ops, o);
                args = args->next;
            }
            ops = List_getFirst(ops);
            fields = List_getFirst(fields);
            while (ops) {
                Ssa_Operand_t o = ops->data;
                Id_t field = (Id_t) fields->data;

                emitStm(Ssa_Stm_new_store
                                (Ssa_Mem_new_class(fresh, field), o));

                fields = fields->next;
                ops = ops->next;
            }
            return Ssa_Operand_new_id(fresh);
        }
        case HIL_EXP_NEW_ARRAY: {
            Id_t fresh = genDec(e->u.newArray.type);
            Ssa_Operand_t newSize = Trans_exp(e->u.newArray.size);

            emitStm(Ssa_Stm_new_newArray
                            (fresh, e->u.newArray.type, newSize));
            return Ssa_Operand_new_id(fresh);
        }
        case HIL_EXP_LVAL: {
            Id_t left = Trans_lvalRight(e->u.lval);

            return Ssa_Operand_new_id(left);;
        }
        case HIL_EXP_CALL: {
            Id_t result = genDec(e->ty);
            List_t args = List_getFirst(e->u.call.args);
            List_t newArgs = List_new();
            Label_t normal = Label_new();

            while (args) {
                Hil_Exp_t current = (Hil_Exp_t) args->data;
                Ssa_Operand_t r = Trans_exp(current);

                List_insertLast(newArgs, r);
                args = args->next;
            }
            emitTrans(Ssa_Transfer_new_call(result, e->u.call.f, newArgs, e->u.call.leave, normal));
            emitLabel(normal);
            return Ssa_Operand_new_id(result);
        }
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

static void Trans_stm(Hil_Stm_t s) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case HIL_STM_ASSIGN: {
            struct Lval_Result_t left
                    = Trans_lvalLeft(s->u.assign.lval);
            Ssa_Operand_t right = Trans_exp(s->u.assign.exp);

            if (left.kind == ID)
                emitStm(Ssa_Stm_new_move(left.u.id, right));
            else if (left.kind == MEM) {
                emitStm(Ssa_Stm_new_store(left.u.mem, right));
                return;
            } else {
                Error_impossible ();
                return;
            }
            return;
        }
        case HIL_STM_EXP: {
            Trans_exp(s->u.exp);
            return;
        }
        case HIL_STM_IF: {
            Ssa_Operand_t cond =
                    Trans_exp(s->u.iff.cond);
            Label_t tl = genLabel(S_IF_TRUE),
                    fl = genLabel(S_IF_FALSE),
                    end = genLabel(S_IF_END);

            emitTrans(Ssa_Transfer_new_if(cond,
                                          tl,
                                          fl));
            emitLabel(tl);
            List_foreach(s->u.iff.then,
                         (Poly_tyVoid) Trans_stm);
            emitTrans(Ssa_Transfer_new_jump(end));
            emitLabel(fl);
            List_foreach(s->u.iff.elsee,
                         (Poly_tyVoid) Trans_stm);
            emitTrans(Ssa_Transfer_new_jump(end));
            emitLabel(end);
            return;
        }
        case HIL_STM_DO: {
            Ssa_Operand_t result;
            Label_t start = genLabel(S_WHILE_START);

            emitLabel(start);
            List_foreach(s->u.doo.body, (Poly_tyVoid) Trans_stm);

            emitTrans(Ssa_Transfer_new_jump(s->u.doo.entryLabel));

            List_foreach(s->u.doo.padding, (Poly_tyVoid) Trans_stm);

            if (Control_labelInfo)
                Property_set(labelInfoProp, s->u.doo.entryLabel, S_WHILE_BC);
            emitLabel(s->u.doo.entryLabel);
            result = Trans_exp(s->u.doo.cond);
            emitTrans(Ssa_Transfer_new_if(result, start, s->u.doo.exitLabel));
            if (Control_labelInfo)
                Property_set(labelInfoProp, s->u.doo.exitLabel, S_WHILE_END);
            emitLabel(s->u.doo.exitLabel);
            return;
        }
        case HIL_STM_JUMP: {
            emitTrans(Ssa_Transfer_new_jump(s->u.jump));
            return;
        }
        case HIL_STM_THROW: {
            emitTrans(Ssa_Transfer_new_throw());
            return;
        }
        case HIL_STM_LOCALTHROW: {
            emitStm(Ssa_Stm_new_try_end(s->u.localThrow));
            //emitTrans (Ssa_Transfer_new_jump (s->u.localThrow));
            return;
        }
        case HIL_STM_TRYCATCH: {
            Label_t end = s->u.tryCatch.end;

            emitStm(Ssa_Stm_new_try(s->u.tryCatch.label));
            List_foreach(s->u.tryCatch.tryy, (Poly_tyVoid) Trans_stm);
            emitStm(Ssa_Stm_new_try_end(end));
            // we will jump here implicitly
            emitTrans(Ssa_Transfer_new_jump(end));
            emitLabel(s->u.tryCatch.label);
            List_foreach(s->u.tryCatch.catchh, (Poly_tyVoid) Trans_stm);
            emitTrans(Ssa_Transfer_new_jump(end));
            emitLabel(end);
            return;
        }
        case HIL_STM_RETURN: {
            Ssa_Operand_t result = Trans_exp(s->u.returnn);

            emitStm(Ssa_Stm_new_move(fun.retId, result));
            emitTrans(Ssa_Transfer_new_jump(fun.exitLabel));
            return;
        }
        default:
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

////////////////////////////////////////////////
// 
static Property_t substProp = 0;
static List_t labelCache = 0;

static void LabelCache_init() {
    substProp = Property_new((Poly_tyPlist) Label_plist);
    labelCache = List_new();
}

static void pushLabel(Label_t l) {
    List_insertLast(labelCache, l);
}

// call this only there is no other labels
static Label_t polluteLabel() {
    List_t p;
    Label_t first, other;

    Assert_ASSERT(labelCache);
    p = List_getFirst(labelCache);
    // if this cache is empty, then we must encounter
    // some dead code. But we rely the optimizor to
    // cut it.
    if (!p) {
        return Label_new();
        return 0;
    }
    first = (Label_t) p->data;
    p = p->next;
    while (p) {
        other = (Label_t) p->data;
        Property_set(substProp, other, first);
        p = p->next;
    }
    // ready for the next round
    labelCache = List_new();
    return first;
}

struct Block_Result_t {
    Label_t newExit;
    List_t blocks;
};

// cook blocks
static struct Block_Result_t cookBlocks() {
    struct Block_Result_t result = {0, 0};
    List_t p = List_getFirst(caches);
    // List<Ssa_Stm_t>
    List_t stms = List_new();
    // List<Ssa_Block_t>
    List_t blocks = List_new();

    LabelCache_init();
    pushLabel(fun.entryLabel);

    while (p) {
        Cache_Data_t data = (Cache_Data_t) p->data;
        switch (data->kind) {
            case LABEL:
                pushLabel(data->u.label);
                break;
            case STM:
                List_insertLast(stms, data->u.stm);
                break;
            case TRANS: {
                // this also has the effect of clearing label cache.
                Label_t firstLabel = polluteLabel();
                Ssa_Transfer_t newTransfer;
                Ssa_Block_t block;

                switch (data->u.trans->kind) {
                    case SSA_TRANS_IF:
                        newTransfer =
                                Ssa_Transfer_renameLabels_if
                                        (data->u.trans, Property_get(substProp,
                                                                     data->u.trans->u.iff.truee),
                                         Property_get(substProp,
                                                      data->u.trans->u.iff.falsee));
                        break;
                    case SSA_TRANS_JUMP:
                        newTransfer =
                                Ssa_Transfer_renameLabels_jump
                                        (data->u.trans, Property_get(substProp,
                                                                     data->u.trans->u.jump));
                        break;
                    case SSA_TRANS_RETURN:
                        newTransfer = data->u.trans;
                        break;
                    case SSA_TRANS_THROW:
                        newTransfer = data->u.trans;
                        break;
                    case SSA_TRANS_CALL:
                        newTransfer = data->u.trans;
                        break;
                    default:
                        Error_impossible ();
                        return result;
                }
                block = Ssa_Block_new(firstLabel, stms, newTransfer);
                List_insertLast(blocks, block);
                stms = List_new();
                break;
            }
            default: {
                Error_impossible ();
                return result;
            }
        }
        p = p->next;
    }
    {
        Label_t tmp = Property_get(substProp, fun.exitLabel);
        result.newExit = tmp ? tmp : fun.exitLabel;
    }
    result.blocks = blocks;
    return result;
}

////////////////////////////////////////////////
// translation of functions
static Ssa_Fun_t Trans_funEach(Hil_Fun_t f) {
    List_t blocks;

    Assert_ASSERT(f);

    caches = List_new();
    allDecs = List_new();

    LabelInfo_init();

    fun.entryLabel = Label_new();
    fun.exitLabel = Label_new();
    fun.retId = genDec(f->type);
    List_foreach(f->stms,
                 (Poly_tyVoid) Trans_stm);
    emitLabel(fun.exitLabel);
    emitTrans(Ssa_Transfer_new_return
                      (Ssa_Operand_new_id(fun.retId)));

    List_append(f->decs, getDecs());

    // the code has been generated by not further cooked.

    {
        List_t p = List_getFirst(f->decs);

        Log_strs("intermediate result in translating to SSA:\n", Atype_toString(f->type), " ", Id_toString(f->name),
                 "\n", "local decs:\n", 0);
        // all local vars
        while (p) {
            Dec_t dec = (Dec_t) p->data;

            Log_fun(dec, (Poly_tyLog) Dec_print);
            Log_str(";");
            p = p->next;
        }
        // all statements
        Cache_log();
    }
    // cook further to pre-SSA form
    {

        struct Block_Result_t result = cookBlocks();
        Log_str("cook pre-SSA form starting:");
        fun.exitLabel = result.newExit;
        blocks = result.blocks;
    }

    // clear properties
    LabelInfo_clear();
    Property_clear(substProp);

    return Ssa_Fun_new(f->type, f->name, f->args, f->decs, blocks, fun.retId, fun.entryLabel, fun.exitLabel);
}

static List_t Trans_funcs(List_t fs) {
    Assert_ASSERT(fs);
    return List_map(fs, (Poly_tyId) Trans_funEach);
}

static void scanFields(Class_t c) {
    Id_t className = c->name;
    List_t fs = List_new();
    List_t decs;

    decs = List_getFirst(c->decs);
    while (decs) {
        Dec_t d = (Dec_t) decs->data;

        List_insertLast(fs, d->id);
        decs = decs->next;
    }
    Property_set(fieldProp, className, fs);
    return;
}

static Ssa_Prog_t Hil_transTraced(Hil_Prog_t p) {
    List_t newFuncs;

    Assert_ASSERT(p);
    fieldProp = Property_newInitFun
            ((Poly_tyPlist) Id_plist, (Poly_tyPropInit) fieldPropInitFun);

    // scan all fields for each class and remember them
    List_foreach(p->classes, (Poly_tyVoid) scanFields);
    // cook functions
    newFuncs = Trans_funcs(p->funcs);
    // clear properties
    Property_clear(fieldProp);
    return Ssa_Prog_new(p->classes, newFuncs);
}

static void printArg(Hil_Prog_t p) {
    File_saveToFile("trans_hil.arg", (Poly_tyPrint) Hil_Prog_print, p);
    return;
}

static void printResult(Ssa_Prog_t p) {
    File_saveToFile("trans_hil.result", (Poly_tyPrint) Ssa_Prog_print, p);
    return;
}

Ssa_Prog_t Hil_trans(Hil_Prog_t p) {
    Ssa_Prog_t r;

    Trace_TRACE("Trans_hil", Hil_transTraced, (p), printArg, r, printResult);

    if (Control_jpg) {
        Ssa_Prog_toDot(r, "preSsa");
    }

    return r;
}
