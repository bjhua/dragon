#include "elaborate.h"
#include "../control/error-msg.h"
#include "../lib/error.h"
#include "../lib/hash.h"
#include "../lib/list.h"
#include "../lib/stack.h"
#include "../lib/string.h"
#include "../lib/trace.h"
#include "../lib/unused.h"
#include "env.h"
#include "type.h"
#include <assert.h>

static void error(String_t, Region_t);

// Used to check the "return" statement. Assigned for
// per-function checking.
static Type_t returnTy = 0;

static void error(String_t msg, Region_t r) {
    ErrorMsg_elabError(msg, r);
}

static void error_mismatch(Type_t wanted, Type_t got,
                           Region_t r) {
    error(String_concat("type mismatch"
                        "\nexpects: ",
                        Type_toString(wanted),
                        "\nbut got: ",
                        Type_toString(got),
                        0),
          r);
}

static void checkType(Type_t t1, Type_t t2, Region_t r) {
    if (!Type_equals(t1, t2))
        error_mismatch(t1, t2, r);
}

struct Lval_Result_t {
    Type_t type;
    Ast_Lval_t lval;
};

struct Exp_Result_t {
    Type_t type;
    Ast_Exp_t exp;
};

static Ast_Type_t Elab_convertType(Ast_Type_t);

static Type_t Elab_semType(Ast_Type_t);

static Ast_Block_t Elab_block(Ast_Block_t);

static struct Lval_Result_t Elab_lval(Ast_Lval_t);

static struct Exp_Result_t Elab_exp(Ast_Exp_t);


/////////////////////////////////////////////////////
// left-values
/////////////////////////////////////////////////////
static struct Lval_Result_t Elab_lval(Ast_Lval_t l) {
    struct Lval_Result_t result = {Type_new_int(), l};

    assert(l);
    switch (l->kind) {
        case AST_LVAL_VAR: {
            Env_Binding_t bd;

            bd = Venv_lookupMustExist(l->u.var);
            result.type = bd->type;
            result.lval = Ast_Lval_new_var(bd->fresh, result.type, l->region);
            return result;
        }
        case AST_LVAL_DOT: {
            Env_Binding_t bdVar;
            struct Lval_Result_t left;
            Type_t tyLeft;
            AstId_t className;

            left = Elab_lval(l->u.dot.lval);
            tyLeft = left.type;

            if (tyLeft->isArray) {
                error("want a class type, but got array: ",
                      l->region);
                return result;
            }
            if (tyLeft->kind != TYPE_A_CLASS) {
                error("want a class type: ",
                      l->region);
                return result;
            }
            className = tyLeft->u.className;
            bdVar = Senv_lookupMustExist(className, l->u.dot.var);
            result.type = bdVar->type;
            result.lval = Ast_Lval_new_dot(left.lval, bdVar->fresh, result.type, l->region);
            return result;
        }
        case AST_LVAL_ARRAY: {
            struct Lval_Result_t r;
            struct Exp_Result_t re;

            r = Elab_lval(l->u.array.lval);
            re = Elab_exp(l->u.array.exp);

            checkType(Type_new_int(), re.type, l->region);
            if (!r.type->isArray) {
                printf("expected: array type\n"
                       "but got : %s",
                       Type_toString(r.type));
                error("", l->region);
                result.type = Type_new_int();
            } else
                result.type = Type_clearArray(r.type);
            result.lval = Ast_Lval_new_array(r.lval, re.exp, result.type, 0);
            return result;
        }
        default:
            Error_impossible();
            return result;
    }
    Error_impossible();
    return result;
}


/////////////////////////////////////////////////////
// expressions
/////////////////////////////////////////////////////
static struct Exp_Result_t Elab_exp(Ast_Exp_t e) {
    struct Exp_Result_t result = {Type_new_int(), e};

    assert(e);
    switch (e->kind) {
        case AST_EXP_ASSIGN: {
            // For this case, the left expression
            //   left = right
            // should always take the form of a left-value, and
            // this should be checked before, for this is a
            // syntactic error.
            struct Exp_Result_t r1, r2;

            r1 = Elab_exp(e->u.assign.left);
            r2 = Elab_exp(e->u.assign.right);
            checkType(r1.type, r2.type, e->region);
            result.exp = Ast_Exp_new_assign(r1.exp, r2.exp, r1.type, e->region);
            result.type = r1.type;
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
        case AST_EXP_GE: {
            struct Exp_Result_t r1, r2;
            Type_t tint = Type_new_int();

            r1 = Elab_exp(e->u.bop.left);
            checkType(tint, r1.type, e->region);
            r2 = Elab_exp(e->u.bop.right);
            checkType(tint, r2.type, e->region);
            result.exp = Ast_Exp_new_bop(e->kind,
                                         r1.exp,
                                         r2.exp, tint, e->region);
            result.type = tint;
            return result;
        }
        case AST_EXP_EQ:
        case AST_EXP_NE: {
            struct Exp_Result_t r1, r2;
            Type_t ty1, ty2;
            Type_t tint = Type_new_int();

            result.type = Type_new_int();
            result.exp = e;
            r1 = Elab_exp(e->u.bop.left);
            r2 = Elab_exp(e->u.bop.right);
            ty1 = r1.type;
            ty2 = r2.type;
            if (ty1->kind == TYPE_A_STRING || ty2->kind == TYPE_A_STRING) {
                error("string type can not be compared",
                      e->region);
                return result;
            }
            if (ty1->kind == TYPE_A_NS && ty2->kind == TYPE_A_NS)
                return result;
            checkType(ty1, ty2, e->region);
            result.exp = Ast_Exp_new_bop(e->kind, r1.exp, r2.exp, tint, e->region);
            return result;
        }
        case AST_EXP_NOT:
        case AST_EXP_NEGATIVE: {
            struct Exp_Result_t r1;
            Type_t ty, tint = Type_new_int();

            r1 = Elab_exp(e->u.unary.e);
            ty = r1.type;
            checkType(tint, ty, e->region);
            result.type = tint;
            result.exp = Ast_Exp_new_unary(e->kind, r1.exp, tint, e->region);
            return result;
        }
        case AST_EXP_NULL: {
            // should we set this field?
            result.type = Type_new_ns();
            result.exp = e;
            return result;
        }
        case AST_EXP_INTLIT: {
            // should we set this field?
            result.type = Type_new_int();
            result.exp = e;
            e->ty = Type_new_int();
            return result;
        }
        case AST_EXP_STRINGLIT: {
            result.type = Type_new_string();
            result.exp = e;
            e->ty = Type_new_string();
            return result;
        }
        case AST_EXP_NEW_CLASS: {
            AstId_t oldClassName = e->u.newClass.name;
            AstId_t freshClassName =
                    Cenv_lookupMustExist(oldClassName);

            // treat the empty-argument case specially
            if (List_isEmpty(e->u.newClass.args)) {
                result.type = Type_new_class(oldClassName);
                result.exp =
                        Ast_Exp_new_newClass(freshClassName, e->u.newClass.args, result.type);
                return result;
            } else {
                // the normal case, all fields should be available;
                // check each field in turn.
                List_t ptr_field, ptr_arg, newFields = List_new();

                ptr_field = List_getFirst(Cenv_lookupFields(oldClassName));
                ptr_arg = List_getFirst(e->u.newClass.args);

                while (ptr_field && ptr_arg) {
                    AstId_t fid = (AstId_t) ptr_field->data;
                    Ast_Exp_t ae = (Ast_Exp_t) ptr_arg->data;
                    struct Exp_Result_t re = Elab_exp(ae);
                    Env_Binding_t bd =
                            Senv_lookupMustExist(oldClassName,
                                                 fid);
                    assert(bd->type);
                    checkType(bd->type, re.type, e->region);
                    List_insertLast(newFields, re.exp);
                    ptr_field = ptr_field->next;
                    ptr_arg = ptr_arg->next;
                }
                result.type = Type_new_class(e->u.newClass.name);
                result.exp = Ast_Exp_new_newClass(freshClassName, newFields, result.type);
                return result;
            }
        }
        case AST_EXP_NEW_ARRAY: {
            Ast_Type_t newTy =
                    Elab_convertType(e->u.newArray.type);
            Type_t ty = Elab_semType(e->u.newArray.type);
            struct Exp_Result_t r;

            r = Elab_exp(e->u.newArray.size);

            checkType(Type_new_int(), r.type, e->region);

            result.type = ty;
            printf("%s\n", Type_toString(ty));
            result.exp = Ast_Exp_new_newArray(newTy, Type_new_array(ty), r.exp);
            return result;
        }
        case AST_EXP_LVAL: {
            struct Lval_Result_t r;

            r = Elab_lval(e->u.lval);
            result.type = r.type;
            result.exp = Ast_Exp_new_lval(r.lval, r.type);
            return result;
        }
        case AST_EXP_CALL: {
            Env_Binding_t funNameBinding;
            Type_t ty, retTy, argsFrom;
            List_t argsTy, args, argsResult;
            Tuple_t tuple;

            funNameBinding = Venv_lookupMustExist(e->u.call.f);
            ty = funNameBinding->type;
            if (ty->kind == TYPE_A_FUN) {
                tuple = Type_dest_fun(ty);
                argsFrom = Tuple_first(tuple);
                retTy = Tuple_second(tuple);
            } else {
                argsFrom = Type_new_product(Type_new_int(), 0);
                retTy = Type_new_int();
            }
            if (argsFrom->kind != TYPE_A_PRODUCT)
                Error_bug("compiler bug");
            argsTy = List_getFirst(argsFrom->u.product);
            args = List_getFirst(e->u.call.args);
            argsResult = List_new();
            while (argsTy && args) {
                Type_t ty;
                struct Exp_Result_t r;

                ty = (Type_t) argsTy->data;
                r = Elab_exp(args->data);
                List_insertLast(argsResult, r.exp);
                checkType(ty, r.type,
                          AstId_dest(e->u.call.f));
                argsTy = argsTy->next;
                args = args->next;
            }
            if (argsTy || args)
                error("type mismatch in function call",
                      0);
            result.type = retTy;

            result.exp = Ast_Exp_new_call(funNameBinding->fresh, argsResult, retTy);
            return result;
        }
        default:
            Error_impossible();
            return result;
    }
    Error_impossible();
    return result;
}

/////////////////////////////////////////////////////
// statements
/////////////////////////////////////////////////////

static Ast_Stm_t Elab_stm(Ast_Stm_t s) {
    assert(s);

    switch (s->kind) {
        case AST_STM_EXP: {
            struct Exp_Result_t r;

            r = Elab_exp(s->u.exp);
            return Ast_Stm_new_exp(r.exp);
        }
        case AST_STM_IF: {
            struct Exp_Result_t re;
            Ast_Stm_t newThen, newElse;

            re = Elab_exp(s->u.iff.cond);
            checkType(Type_new_int(), re.type, s->region);
            newThen = Elab_stm(s->u.iff.then);
            newElse = Elab_stm(s->u.iff.elsee);
            return Ast_Stm_new_if(re.exp,
                                  newThen,
                                  newElse,
                                  s->region);
        }
        case AST_STM_WHILE: {
            struct Exp_Result_t re;
            Ast_Stm_t newBody;

            re = Elab_exp(s->u.whilee.cond);
            checkType(Type_new_int(), re.type, s->region);
            newBody = Elab_stm(s->u.whilee.body);
            return Ast_Stm_new_while(re.exp, newBody, s->region);
        }
        case AST_STM_DO: {
            struct Exp_Result_t re;
            Ast_Stm_t newBody;

            re = Elab_exp(s->u.doo.cond);
            checkType(Type_new_int(), re.type, s->region);
            newBody = Elab_stm(s->u.doo.body);
            return Ast_Stm_new_do(re.exp, newBody, s->region);
        }
        case AST_STM_FOR: {
            struct Exp_Result_t reHeader, reCond, reTail;
            Ast_Stm_t newBody;

            reHeader = Elab_exp(s->u.forr.header);
            checkType(Type_new_int(), reHeader.type, s->region);
            reCond = Elab_exp(s->u.forr.cond);
            checkType(Type_new_int(), reCond.type, s->region);
            reTail = Elab_exp(s->u.forr.tail);
            checkType(Type_new_int(), reTail.type, s->region);
            newBody = Elab_stm(s->u.forr.body);
            return Ast_Stm_new_for(reHeader.exp,
                                   reCond.exp,
                                   reTail.exp,
                                   newBody,
                                   s->region);
        }
        case AST_STM_BREAK:
            return s;
        case AST_STM_CONTINUE:
            return s;
        case AST_STM_RETURN: {
            struct Exp_Result_t re = Elab_exp(s->u.returnn);
            // a hack!!! but should rewrite this
            re.exp->ty = re.type;
            // the external variable should be properly set up.
            assert(returnTy);
            checkType(returnTy, re.type, s->region);
            return Ast_Stm_new_return(re.exp, s->region);
        }
        case AST_STM_BLOCK:
            return Ast_Stm_new_block(Elab_block(s->u.block));
        case AST_STM_THROW: {
            return s;
        }
        case AST_STM_TRYCATCH: {
            Ast_Stm_t newTry = Elab_stm(s->u.trycatch.tryy);
            Ast_Stm_t newCatch = Elab_stm(s->u.trycatch.catchh);
            return Ast_Stm_new_tryCatch(newTry,
                                        newCatch,
                                        s->region);
        }
        default:
            Error_impossible();
            return s;
    }
    Error_impossible();
    return s;
}

/////////////////////////////////////////////////////
// declarations: function arguments and local vars.
/////////////////////////////////////////////////////
static Ast_Dec_t Elab_dec(Ast_Dec_t t) {
    Ast_Type_t newty;
    Type_t ty;
    AstId_t newName;
    Ast_Exp_t newInit = 0;

    assert(t);

    newty = Elab_convertType(t->type);
    ty = Elab_semType(t->type);
    newName = Venv_insert(t->var, ty);
    if (t->init) {
        struct Exp_Result_t r = Elab_exp(t->init);
        Type_t ety = r.type;
        newInit = r.exp;
        checkType(ty, ety, AstId_getRegion(t->var));
    }
    return Ast_Dec_new(newty, newName, newInit);
}

/////////////////////////////////////////////////////
// Elaboration of blocks
/////////////////////////////////////////////////////

struct Top_Block_Result_t {
    List_t args;
    List_t decs;
    List_t stms;
};

static Ast_Block_t Elab_block(Ast_Block_t b) {
    List_t decs, stms;

    assert(b);
    Venv_enterScope();
    decs = List_map(b->decs, (Poly_tyId) Elab_dec);
    stms = List_map(b->stms, (Poly_tyId) Elab_stm);
    Venv_exitScope();
    return Ast_Block_new(decs, stms);
}

static struct Top_Block_Result_t
Elab_topBlock(List_t args, Ast_Block_t b) {
    struct Top_Block_Result_t result;

    assert(args);
    assert(b);

    Venv_enterScope();
    result.args = List_map(args, (Poly_tyId) Elab_dec);
    result.decs = List_map(b->decs, (Poly_tyId) Elab_dec);
    result.stms = List_map(b->stms, (Poly_tyId) Elab_stm);
    Venv_exitScope();
    return result;
}

/////////////////////////////////////////////////////
// Elaboration of functions
/////////////////////////////////////////////////////

static Ast_Fun_t Elab_funPass2Each(Ast_Fun_t f) {
    Env_Binding_t bdForF;
    struct Top_Block_Result_t b;
    AstId_t newFunName;

    assert(f);

    bdForF = Venv_lookupMustExist(f->name);
    // before elaborating function body, we must record
    // function return type, for it will be used to
    // check return statement.
    returnTy = Elab_semType(f->type);

    // The top block is a little special in that it also
    // include the function argument, i.e., function
    // arguments and locals are in the same scope (scope 1.)
    b = Elab_topBlock(f->args, f->block);
    newFunName =
            (AstId_equals(AstId_fromString("dragon", 0),
                          f->name))
                    ? f->name
                    : bdForF->fresh;
    return Ast_Fun_new(Elab_convertType(f->type),
                       newFunName,
                       b.args,
                       Ast_Block_new(b.decs,
                                     b.stms),
                       f->region);
}

static void Elab_funPass1Each(Ast_Fun_t f) {
    Type_t ty_ret, ty_arg;
    List_t tylist = List_new(), tmp;

    assert(f);

    // Cook the return type. It's a semantic type.
    ty_ret = Elab_semType(f->type);
    // Cook the argument type. It's a semantic type.
    tmp = f->args->next;
    while (tmp) {
        Ast_Dec_t dec = (Ast_Dec_t) tmp->data;
        Type_t ty = Elab_semType(dec->type);
        List_insertLast(tylist, ty);
        tmp = tmp->next;
    }
    ty_arg = Type_new_product2(tylist);

    // enter this function type into Venv:
    Venv_insert(f->name, Type_new_fun(ty_arg, ty_ret));
    return;
}

static List_t Elab_funs(List_t fs) {
    List_t newFuncs;

    assert(fs);

    Venv_init();

    // In the 1st round, collect all function types and
    // put them in the Venv table, these functions are
    // at scope 0 (out-most).
    List_foreach(fs, (Poly_tyVoid) Elab_funPass1Each);
    // In the 2nd round of checking, each function is
    // checked and converted to a fresh function.
    newFuncs = List_map(fs, (Poly_tyId) Elab_funPass2Each);
    return newFuncs;
}

/////////////////////////////////////////////////////
// Elaboration of classes
/////////////////////////////////////////////////////
static Ast_Type_t Elab_convertType(Ast_Type_t ty) {
    assert(ty);
    switch (ty->kind) {
        case AST_TYPE_ID: {
            AstId_t fresh = Cenv_lookupMustExist(ty->id);
            Ast_Type_t newTy = Ast_Type_new_id(fresh);

            if (ty->isArray)
                Ast_Type_setArray(newTy);
            return newTy;
        }
        default: {
            return ty;
        }
    }
}

static Type_t Elab_semType(Ast_Type_t ty) {
    Type_t new;

    assert(ty);

    switch (ty->kind) {
        case AST_TYPE_INT:
            new = Type_new_int();
            break;
        case AST_TYPE_STRING:
            new = Type_new_string();
            break;
        case AST_TYPE_ID:
            // use the old type name
            new = Type_new_class(ty->id);
            break;
        default:
            printf("%d\n", ty->kind);
            Error_impossible();
            return 0;
    }
    if (ty->isArray) {
        Type_set_array(new);
    }
    return new;
}

static AstId_t oldClassName = 0;

static Ast_Dec_t Elab_fieldEach(Ast_Dec_t dec) {
    AstId_t newFieldName = 0;
    Ast_Type_t newType;
    Type_t semType;

    assert(dec);

    // as we're converting an old ast to a new ast, then
    // we must ensure the type consistence.
    newType = Elab_convertType(dec->type);
    // env should store semantic type, not syntax type.
    semType = Elab_semType(dec->type);
    newFieldName = Senv_insert(oldClassName, dec->var, semType);
    // check the "init" field to be 0
    assert(dec->init == 0);
    return Ast_Dec_new(newType, newFieldName, dec->init);
}

// Elaborate each class in turn. Along the way, renaming
// all variables and insert all fields into "Senv".
static Ast_Class_t Elab_classPass2Each(Ast_Class_t c) {
    AstId_t newClassName;
    List_t newFields;

    assert(c);
    oldClassName = c->name;
    newClassName = Cenv_lookupMustExist(c->name);
    newFields = List_map(c->fields, (Poly_tyId) Elab_fieldEach);
    return Ast_Class_new(newClassName, newFields);
}

static List_t Elab_classPass2(List_t classes) {
    return List_map(classes,
                    (Poly_tyId) Elab_classPass2Each);
}

// Insert each class's name into "Tenv". Note this has
// the effect of assigning fresh names to class names.
static void Elab_classPass1Each(Ast_Class_t class) {
    List_t p, fields = List_new();

    assert(class);

    p = List_getFirst(class->fields);
    while (p) {
        Ast_Dec_t d = (Ast_Dec_t) p->data;

        List_insertLast(fields, d->var);
        p = p->next;
    }
    Cenv_insert(class->name, fields);
    //    return;
}

// Elaborate each class in turn.
static void Elab_classPass1(List_t classes) {
    List_foreach(classes, (Poly_tyVoid) Elab_classPass1Each);
}

// The elaboration pass of classes consists of two
// sub-passes:
//   1. scan all classes, and gather all class names
//      and then put these names into the symbol table
//      "Cenv".
//   2. use the table "Tenv" formed in pass 1 to scan each
//      class in turn again.
static List_t Elab_classes_traced(List_t classes) {
    List_t newClasses;

    assert(classes);

    // collect all class names.
    Elab_classPass1(classes);
    // Now, the "Cenv" should map all class names to
    // fresh new ones. And we scan each class' body.
    newClasses = Elab_classPass2(classes);
    return newClasses;
}

static void Trace_arg_class(List_t classes) {
    UNUSED(classes);

    //Ast_Prog_print (p, stdout);
    //    return;
}

static void Trace_result_class(List_t classes) {
    UNUSED(classes);
    //Ast_Prog_print (p, stdout);
    //    return;
}

static List_t Elab_classes(List_t classes) {
    List_t r;

    Trace_TRACE("elab-class", Elab_classes_traced, (classes), Trace_arg_class, r, Trace_result_class);
    return r;
}

/////////////////////////////////////////////////////
// Elaboration of programs
/////////////////////////////////////////////////////
static Ast_Prog_t Elaborate_ast_traced(Ast_Prog_t p) {
    List_t newClasses, newFuncs;

    assert(p);
    // before doing anything, first initialize
    // class env "Tenv" and class field env "Senv".
    Cenv_init();
    Senv_init();

    newClasses = Elab_classes(p->classes);
    newFuncs = Elab_funs(p->funcs);

    // construct a new AST iff no error found.
    ErrorMsg_errorExit();
    return Ast_Prog_new(newClasses, newFuncs);
}

static void printArg(Ast_Prog_t p) {
    File_saveToFile("elab.arg", (Poly_tyPrint) Ast_Prog_print, p);
    return;
}

static void printResult(Ast_Prog_t p) {
    File_saveToFile("elab.result", (Poly_tyPrint) Ast_Prog_print, p);
    return;
}

Ast_Prog_t Elaborate_ast(Ast_Prog_t p) {
    Ast_Prog_t r;

    Trace_TRACE("Elab_ast", Elaborate_ast_traced, (p), printArg, r, printResult);
    return r;
}
