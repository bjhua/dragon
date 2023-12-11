#include "trans-ast.h"
#include "../atoms/atoms.h"
#include "../lib/app-list.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/property.h"
#include "../lib/stack.h"
#include "../lib/string.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"
#include <assert.h>

// Data structures to hold labels:
//   labels are loop labels which jump targets for "break" and "continue".
//   exnLabels are labels for local "throw" (statically nested "throw").
static Stack_t labels = 0;
static Stack_t exnLabels = 0;

static Tuple_t pushLabel() {
    Label_t entryLabel = Label_new(),
            exitLabel = Label_new();

    assert(labels);
    Stack_push(labels, Tuple_new(entryLabel, exitLabel));
    return Tuple_new(entryLabel, exitLabel);
}

static void popLabel() {
    assert(labels);
    Stack_pop(labels);
}

static Tuple_t peekLabel() {
    assert(labels);
    return Stack_getTop(labels);
}

static Label_t pushExnLabel() {
    Label_t label = Label_new();

    assert(exnLabels);
    Stack_push(exnLabels, label);
    return label;
}

static void popExnLabel() {
    assert(exnLabels);
    Stack_pop(exnLabels);
}

static Label_t peekExnLabel() {
    assert(exnLabels);
    return Stack_getTop(exnLabels);
}

/***************************************************************/

struct Lval_Result_t {
    AppList_t list;
    Hil_Lval_t lval;
};

struct Exp_Result_t {
    AppList_t list;
    Hil_Exp_t exp;
};

static List_t Elab_block(Ast_Block_t);

static struct Lval_Result_t Elab_lval(Ast_Lval_t);

static struct Exp_Result_t Elab_exp(Ast_Exp_t);

static List_t Elab_stm(Ast_Stm_t);


///////////////////////////////////////////////////////
// translation of semantic "elaborate/type" to "atom/atype"
static Atype_t type2atype(Type_t ty) {
    assert(ty);
    switch (ty->kind) {
        case TYPE_A_INT:
            if (ty->isArray)
                return Atype_new_int_array();
            return Atype_new_int();
        case TYPE_A_STRING:
            if (ty->isArray)
                return Atype_new_string("");
            return Atype_new_string_array();
        case TYPE_A_NS:
            if (ty->isArray)
                Error_impossible();
            return Atype_new_int();
        case TYPE_A_CLASS: {
            Id_t id = AstId_toId(ty->u.className);

            if (ty->isArray)
                return Atype_new_class_array(id);
            return Atype_new_class(id);
        }
        case TYPE_A_PRODUCT:
            Error_impossible();
            return 0;
        case TYPE_A_FUN: {
            Type_t from = ty->u.fun.from;

            if (from->kind != TYPE_A_PRODUCT) {
                Error_impossible();
                return 0;
            }
            return Atype_new_fun(List_map(from->u.product, (Poly_tyId) type2atype),
                                 type2atype(ty->u.fun.to));
        }
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

///////////////////////////////////////////////////////
// translation of ast "type" to "atom/atype"
static Atype_t asttype2atype(Ast_Type_t ty) {
    assert(ty);
    switch (ty->kind) {
        case AST_TYPE_INT: {
            if (ty->isArray)
                return Atype_new_int_array();
            return Atype_new_int();
        }
        case AST_TYPE_STRING: {
            if (ty->isArray)
                return Atype_new_string_array();
            return Atype_new_string("");
        }
        case AST_TYPE_ID: {
            if (ty->isArray)
                return Atype_new_class_array(AstId_toId(ty->id));
            return Atype_new_class(AstId_toId(ty->id));
        }
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

///////////////////////////////////////////////////////
// translation of left-value
static struct Lval_Result_t Elab_lval(Ast_Lval_t l) {
    struct Lval_Result_t result = {0, 0};

    assert(l);
    switch (l->kind) {
        case AST_LVAL_VAR: {
            Id_t newId = AstId_toId(l->u.var);
            // this is the semantic type
            Atype_t newTy;

            newTy = type2atype(l->ty);


            result.lval = Hil_Lval_new_var(newId, newTy);
            result.list = AppList_new_empty();
            return result;
        }
        case AST_LVAL_DOT: {
            Id_t newVar = AstId_toId(l->u.dot.var);
            Atype_t newTy = type2atype(l->ty);
            struct Lval_Result_t left = Elab_lval(l->u.dot.lval);
            result.lval =
                    Hil_Lval_new_dot(left.lval,
                                     newVar,
                                     newTy);
            result.list = left.list;
            return result;
        }
        case AST_LVAL_ARRAY: {
            struct Lval_Result_t left =
                    Elab_lval(l->u.array.lval);
            struct Exp_Result_t right =
                    Elab_exp(l->u.array.exp);
            Atype_t newTy = type2atype(l->ty);
            result.lval =
                    Hil_Lval_new_array(left.lval,
                                       right.exp,
                                       newTy);
            result.list = AppList_concat(left.list, right.list);
            return result;
        }
        default:
            Error_impossible();
            return result;
    }
    Error_impossible();
    return result;
}

static Operator_t convertOperator(Ast_Exp_Kind_t k) {
    switch (k) {
        case AST_EXP_ADD:
            return OP_ADD;
        case AST_EXP_SUB:
            return OP_SUB;
        case AST_EXP_TIMES:
            return OP_TIMES;
        case AST_EXP_DIVIDE:
            return OP_DIVIDE;
        case AST_EXP_MODUS:
            return OP_MODUS;
        case AST_EXP_OR:
            return OP_OR;
        case AST_EXP_AND:
            return OP_AND;
        case AST_EXP_EQ:
            return OP_EQ;
        case AST_EXP_NE:
            return OP_NE;
        case AST_EXP_LT:
            return OP_LT;
        case AST_EXP_LE:
            return OP_LE;
        case AST_EXP_GT:
            return OP_GT;
        case AST_EXP_GE:
            return OP_GE;
        case AST_EXP_NOT:
            return OP_NOT;
        case AST_EXP_NEGATIVE:
            return OP_NEG;
        default:
            Error_impossible();
            return 0;
    };
    Error_impossible();
    return 0;
}

///////////////////////////////////////////////////////
// translation of expressions
static struct Exp_Result_t Elab_exp(Ast_Exp_t e) {
    struct Exp_Result_t result = {0, 0};

    assert(e);
    switch (e->kind) {
        case AST_EXP_ASSIGN: {
            struct Exp_Result_t newLeft, newRight;
            Hil_Stm_t newAssign;

            newLeft = Elab_exp(e->u.assign.left);
            newRight = Elab_exp(e->u.assign.right);
            result.exp = newLeft.exp;
            if (result.exp->kind != HIL_EXP_LVAL)
                Error_impossible();

            newAssign = Hil_Stm_new_assign(newLeft.exp->u.lval, newRight.exp);
            result.list =
                    AppList_new_list(List_list(newRight.list, newLeft.list, AppList_fromItem(newAssign), 0));
            return result;
        }
        case AST_EXP_ADD:
        case AST_EXP_SUB:
        case AST_EXP_TIMES:
        case AST_EXP_DIVIDE:
        case AST_EXP_MODUS:
        case AST_EXP_OR:
        case AST_EXP_AND:
        case AST_EXP_LT:
        case AST_EXP_LE:
        case AST_EXP_GT:
        case AST_EXP_GE:
        case AST_EXP_EQ:
        case AST_EXP_NE: {
            struct Exp_Result_t newLeft, newRight;
            newLeft = Elab_exp(e->u.bop.left);
            newRight = Elab_exp(e->u.bop.right);

            result.exp =
                    Hil_Exp_new_bop(convertOperator(e->kind), newLeft.exp, newRight.exp, type2atype(e->ty));
            result.list = AppList_concat(newLeft.list, newRight.list);
            return result;
        }
        case AST_EXP_NOT:
        case AST_EXP_NEGATIVE: {
            struct Exp_Result_t new;

            new = Elab_exp(e->u.unary.e);

            result.exp = Hil_Exp_new_unary(convertOperator(e->kind), new.exp, type2atype(e->ty));
            result.list = new.list;
            return result;
        }
        case AST_EXP_NULL: {
            result.exp = Hil_Exp_new_intlit(0, Atype_new_int());
            result.list = AppList_new_empty();
            return result;
        }
        case AST_EXP_INTLIT: {
            result.exp = Hil_Exp_new_intlit(e->u.intlit, Atype_new_int());
            result.list = AppList_new_empty();
            return result;
        }
        case AST_EXP_STRINGLIT: {
            result.exp = Hil_Exp_new_stringlit(e->u.stringlit, Atype_new_string(""));
            result.list = AppList_new_empty();
            return result;
        }
        case AST_EXP_NEW_CLASS: {
            AstId_t className = e->u.newClass.name;
            List_t args = e->u.newClass.args;
            List_t newArgs = List_new();
            List_t stms = List_new();

            args = List_getFirst(args);
            while (args) {
                Ast_Exp_t e = (Ast_Exp_t) args->data;
                struct Exp_Result_t temp = Elab_exp(e);

                List_insertLast(newArgs, temp.exp);
                List_insertLast(stms, temp.list);
                args = args->next;
            }
            result.exp = Hil_Exp_new_newClass(AstId_toId(className), newArgs);
            result.list = AppList_new_list(stms);
            return result;
        }
        case AST_EXP_NEW_ARRAY: {
            struct Exp_Result_t newSize;

            newSize = Elab_exp(e->u.newArray.size);
            result.exp =
                    Hil_Exp_new_newArray(asttype2atype(e->u.newArray.type),
                                         newSize.exp);
            result.list = newSize.list;
            return result;
        }
        case AST_EXP_LVAL: {
            struct Lval_Result_t newLeft;

            newLeft = Elab_lval(e->u.lval);
            result.exp = Hil_Exp_new_lval(newLeft.lval, type2atype(e->u.lval->ty));
            result.list = newLeft.list;
            return result;
        }
        case AST_EXP_CALL: {
            struct Exp_Result_t temp;
            List_t tempExps = List_new();
            List_t tempList = List_new();
            Id_t fname = AstId_toId(e->u.call.f);
            Atype_t ty = type2atype(e->ty);
            List_t args = List_getFirst(e->u.call.args);
            Label_t leave = 0, normal = 0;

            while (args) {
                Ast_Exp_t arg = (Ast_Exp_t) args->data;

                temp = Elab_exp(arg);
                List_insertLast(tempExps, temp.exp);
                List_insertLast(tempList, temp.list);
                args = args->next;
            }
            if (!Stack_isEmpty(exnLabels))
                leave = peekExnLabel();

            result.exp = Hil_Exp_new_call(fname, tempExps, ty, leave, normal);
            result.list = AppList_new_list(tempList);
            return result;
        }
        default:
            Error_impossible();
            return result;
    }
    Error_impossible();
    return result;
}

///////////////////////////////////////////////////////
// translation of statements
// Result: List<Hil_Stm_t>
static List_t Elab_stm(Ast_Stm_t s) {
    List_t result = 0;

    assert(s);
    switch (s->kind) {
        case AST_STM_EXP: {
            struct Exp_Result_t er;

            er = Elab_exp(s->u.exp);
            result = AppList_toList(er.list);
            List_insertLast(result, Hil_Stm_new_exp(er.exp));
            return result;
        }
        case AST_STM_IF: {
            struct Exp_Result_t condresult;
            Hil_Stm_t newStm;

            condresult = Elab_exp(s->u.iff.cond);
            newStm = Hil_Stm_new_if(condresult.exp,
                                    Elab_stm(s->u.iff.then),
                                    Elab_stm(s->u.iff.elsee));
            result = AppList_toList(condresult.list);
            List_insertLast(result, newStm);
            return result;
        }
        case AST_STM_WHILE: {
            Ast_Stm_t new =
                    Ast_Stm_new_if(s->u.whilee.cond,
                                   Ast_Stm_new_do(s->u.whilee.cond,
                                                  s->u.whilee.body,
                                                  s->region),
                                   Ast_Stm_new_block(Ast_Block_new(List_new(),
                                                                   List_new())),
                                   s->region);
            return Elab_stm(new);
        }
        case AST_STM_DO: {
            Tuple_t tuple = pushLabel();
            struct Exp_Result_t expresult;
            Hil_Stm_t newStm;

            expresult = Elab_exp(s->u.doo.cond);
            newStm =
                    Hil_Stm_new_do(expresult.exp, Elab_stm(s->u.doo.body), Tuple_first(tuple), Tuple_second(tuple),
                                   List_new());
            popLabel();
            result = AppList_toList(expresult.list);
            List_insertLast(result, newStm);
            return result;
        }
        case AST_STM_FOR: {
            // for (header; cond; tail)
            //   body;
            Tuple_t tuple = pushLabel();
            struct Exp_Result_t newHeader = Elab_exp(s->u.forr.header);
            List_t newBody;
            struct Exp_Result_t newCond = Elab_exp(s->u.forr.cond);
            List_t newCondx = AppList_toList(newCond.list);
            struct Exp_Result_t newTail;
            List_t newTailx;
            List_t result = List_new();

            newBody = Elab_stm(s->u.forr.body);
            newTail = Elab_exp(s->u.forr.tail);
            newTailx = AppList_toList(newTail.list);
            List_insertLast(newTailx, Hil_Stm_new_exp(newTail.exp));
            List_append(newTailx, newCondx);

            List_append(result, AppList_toList(newHeader.list));
            List_insertLast(result, Hil_Stm_new_exp(newHeader.exp));
            List_append(result, AppList_toList(newCond.list));
            List_insertLast(result, Hil_Stm_new_if(newCond.exp, List_list(Hil_Stm_new_do(newCond.exp, newBody, Tuple_first(tuple), Tuple_second(tuple), newTailx), 0),
                                                   List_new()));
            popLabel();
            return result;
        }
        case AST_STM_BREAK: {
            Tuple_t tuple = peekLabel();
            Label_t exitLabel = Tuple_second(tuple);
            return List_list(Hil_Stm_new_jump(exitLabel), 0);
        }
        case AST_STM_CONTINUE: {
            Tuple_t tuple = peekLabel();
            Label_t entryLabel = Tuple_first(tuple);
            return List_list(Hil_Stm_new_jump(entryLabel), 0);
        }
        case AST_STM_RETURN: {
            struct Exp_Result_t er;
            Hil_Stm_t newStm;

            er = Elab_exp(s->u.returnn);
            newStm = Hil_Stm_new_return(er.exp);
            result = AppList_toList(er.list);
            List_insertLast(result, newStm);
            return result;
        }
            // If this is a local throw, then the control flow
            // tends to be more accurate.
        case AST_STM_THROW: {
            Label_t label;

            if (Stack_isEmpty(exnLabels)) {
                return List_list(Hil_Stm_new_throw(), 0);
            }
            label = peekExnLabel();
            return List_list(Hil_Stm_new_localThrow(label), 0);
        }
        case AST_STM_TRYCATCH: {
            Label_t label;
            List_t newTry, newCatch;

            label = pushExnLabel();
            newTry = Elab_stm(s->u.trycatch.tryy);
            popExnLabel();
            newCatch = Elab_stm(s->u.trycatch.catchh);
            return List_list(Hil_Stm_new_tryCatch(newTry, newCatch, label, Label_new()), 0);
        }
        case AST_STM_BLOCK:
            return Elab_block(s->u.block);
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

///////////////////////////////////////////////////////
// translation of block
static List_t Elab_block(Ast_Block_t b) {
    // result: List<List<Hil_Stm_t>>
    // stms:   List<Hil_Stm_t>
    //
    List_t result, p, stms = List_new();

    assert(b);
    result = List_map(b->stms, (Poly_tyId) Elab_stm);
    p = List_getFirst(result);
    while (p) {
        List_t tmp;

        tmp = (List_t) p->data;
        tmp = List_getFirst(tmp);
        while (tmp) {
            List_insertLast(stms, tmp->data);
            tmp = tmp->next;
        }
        p = p->next;
    }
    return stms;
}

///////////////////////////////////////////////////////
// translation of declaration
static Dec_t Elab_dec(Ast_Dec_t dec) {
    assert(dec);
    return Dec_new(asttype2atype(dec->type), AstId_toId(dec->var));
}

///////////////////////////////////////////////////////
// translation of function
static Hil_Fun_t Elab_funEach(Ast_Fun_t f) {
    List_t newStms;
    Id_t newName;
    Atype_t newType;

    assert(f);
    newName = AstId_toId(f->name);
    newType = asttype2atype(f->type);

    newStms = Elab_block(f->block);
    return Hil_Fun_new(newType, newName, List_map(f->args, (Poly_tyId) Elab_dec),
                       List_map(f->block->decs, (Poly_tyId) Elab_dec), newStms);
}

static List_t Elab_fun(List_t fs) {
    List_t newList;

    assert(fs);
    newList = List_map(fs, (Poly_tyId) Elab_funEach);
    return newList;
}

/////////////////////////////////////////////////////////
// translation of classes
static Class_t Elab_classEach(Ast_Class_t c) {
    return Class_new(AstId_toId(c->name), List_map(c->fields, (Poly_tyId) Elab_dec));
}

static List_t Elab_classes(List_t classes) {
    return List_map(classes, (Poly_tyId) Elab_classEach);
}

/////////////////////////////////////////////////////////
// translation of programs
static Hil_Prog_t Elaborate_ast_traced(Ast_Prog_t p) {
    List_t newClasses, newFuncs;

    assert(p);
    // init the two groups of labels
    labels = Stack_new();
    exnLabels = Stack_new();

    newClasses = Elab_classes(p->classes);
    newFuncs = Elab_fun(p->funcs);
    return Hil_Prog_new(newClasses, newFuncs);
}

static void printArg(Ast_Prog_t p) {
    File_saveToFile("trans-ast.arg", (Poly_tyPrint) Ast_Prog_print, p);
    return;
}

static void printResult(Hil_Prog_t p) {
    File_saveToFile("trans-ast.result", (Poly_tyPrint) Hil_Prog_print, p);
    return;
}

Hil_Prog_t Trans_ast(Ast_Prog_t p) {
    Hil_Prog_t r;

    Trace_TRACE("Trans_ast", Elaborate_ast_traced, (p), printArg, r, printResult);
    return r;
}
