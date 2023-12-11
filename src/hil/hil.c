#include "hil.h"
#include "../control/control.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include <assert.h>

#define E Hil_Exp_t
#define F Hil_Fun_t
#define L Hil_Lval_t
#define P Hil_Prog_t
#define S Hil_Stm_t


/////////////////////////////////////////////////////
// lval
L Hil_Lval_new_var(Id_t var, Atype_t ty) {
    L l;
    Mem_NEW(l);
    l->kind = HIL_LVAL_VAR;
    l->u.var = var;
    l->ty = ty;
    return l;
}

L Hil_Lval_new_dot(L lval, Id_t var, Atype_t ty) {
    L l;
    Mem_NEW(l);
    l->kind = HIL_LVAL_DOT;
    l->u.dot.lval = lval;
    l->u.dot.var = var;
    l->ty = ty;
    return l;
}

L Hil_Lval_new_array(L lval, E e, Atype_t ty) {
    L l;
    Mem_NEW(l);
    l->kind = HIL_LVAL_ARRAY;
    l->u.array.lval = lval;
    l->u.array.exp = e;
    l->ty = ty;
    return l;
}

File_t Hil_Lval_print(File_t file, L l) {
    assert(l);

    switch (l->kind) {
        case HIL_LVAL_VAR:
            fprintf(file, "%s", Id_toString(l->u.var));
            break;
        case HIL_LVAL_DOT:
            Hil_Lval_print(file, l->u.dot.lval);
            fprintf(file, ".");
            fprintf(file, "%s", Id_toString(l->u.dot.var));
            break;
        case HIL_LVAL_ARRAY:
            Hil_Lval_print(file, l->u.array.lval);
            fprintf(file, "[");
            Hil_Exp_print(file, l->u.array.exp);
            fprintf(file, "]");
            break;
        default:
            Error_impossible();
            return file;
    }
    if (Control_showType)
        if (l->ty)
            fprintf(file, "@: %s @", Atype_toString(l->ty));
        else
            fprintf(file, "@NO_TYPE@");
    else
        ;
    return file;
}

//////////////////////////////////////////////////////
/* expressions */
E Hil_Exp_new_bop(Operator_t op,
                  E left, E right, Atype_t ty) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_BOP;
    e->u.bop.left = left;
    e->u.bop.right = right;
    e->u.bop.op = op;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_unary(Operator_t op, E x, Atype_t ty) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_UOP;
    e->u.unary.e = x;
    e->u.unary.op = op;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_intlit(long i, Atype_t ty) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_INTLIT;
    e->u.intlit = i;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_stringlit(String_t s, Atype_t ty) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_STRINGLIT;
    e->u.stringlit = s;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_newArray(Atype_t ty, E size) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_NEW_ARRAY;
    e->u.newArray.type = ty;
    e->u.newArray.size = size;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_newClass(Id_t name, List_t args) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_NEW_CLASS;
    e->u.newClass.name = name;
    e->u.newClass.args = args;
    e->ty = Atype_new_string("<name>");
    return e;
}

E Hil_Exp_new_lval(L lval, Atype_t ty) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_LVAL;
    e->u.lval = lval;
    e->ty = ty;
    return e;
}

E Hil_Exp_new_call(Id_t f, List_t args, Atype_t ty, Label_t leave, Label_t normal) {
    E e;
    Mem_NEW(e);
    e->kind = HIL_EXP_CALL;
    e->u.call.f = f;
    e->u.call.args = args;
    e->ty = ty;
    e->u.call.leave = leave;
    e->u.call.normal = normal;
    return e;
}

File_t Hil_Exp_print(File_t file, E e) {
    assert(e);
    switch (e->kind) {
        case HIL_EXP_BOP:
            Hil_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", Operator_toString(e->u.bop.op));
            Hil_Exp_print(file, e->u.bop.right);
            break;
        case HIL_EXP_UOP:
            fprintf(file, "%s", Operator_toString(e->u.unary.op));
            Hil_Exp_print(file, e->u.unary.e);
            break;
        case HIL_EXP_INTLIT:
            fprintf(file, "%s", Int_toString(e->u.intlit));
            break;
        case HIL_EXP_STRINGLIT:
            fprintf(file, "\"");
            fprintf(file, "%s", e->u.stringlit);
            fprintf(file, "\"");
            break;
        case HIL_EXP_NEW_CLASS:
            fprintf(file, "%s", "new ");
            fprintf(file, "%s", Id_toString(e->u.newClass.name));
            fprintf(file, "%s", "(");
            List_foldl(e->u.newClass.args, file, (Poly_tyFold) Hil_Exp_print);
            fprintf(file, "%s", ");");
            break;
        case HIL_EXP_NEW_ARRAY:
            fprintf(file, "%s", "new ");
            fprintf(file, "%s", Atype_toString(e->u.newArray.type));
            fprintf(file, "%s", "[");
            Hil_Exp_print(file, e->u.newArray.size);
            fprintf(file, "%s", "]");
            break;
        case HIL_EXP_CALL:
            fprintf(file, "%s", Id_toString(e->u.call.f));
            fprintf(file, "%s", "(");
            List_foldl(e->u.call.args, file, (Poly_tyFold) Hil_Exp_print);
            fprintf(file, "%s", ")");
            fprintf(file, "LEAVE = %s, NORMAL = %s", (e->u.call.leave) ? Label_toString(e->u.call.leave) : "<LEAVE>", (e->u.call.normal) ? Label_toString(e->u.call.normal) : "<NORMAL>");
            break;
        case HIL_EXP_LVAL:
            Hil_Lval_print(file, e->u.lval);
            break;
        default:
            fprintf(stderr, "%d", e->kind);
            Error_impossible();
            break;
    }
    if (Control_showType)
        if (e->ty)
            fprintf(file, "@: %s @", Atype_toString(e->ty));
        else
            fprintf(file, "@NO_TYPE@");
    else
        ;
    return file;
}

//////////////////////////////////////////////////////
/* statements */
S Hil_Stm_new_assign(L lval, E e) {
    S s;
    Mem_NEW(s);
    s->kind = HIL_STM_ASSIGN;
    s->u.assign.lval = lval;
    s->u.assign.exp = e;
    return s;
}

S Hil_Stm_new_exp(E e) {
    S s;
    Mem_NEW(s);
    s->kind = HIL_STM_EXP;
    s->u.exp = e;
    return s;
}

S Hil_Stm_new_if(E e, List_t t, List_t f) {
    S s;
    Mem_NEW(s);
    s->kind = HIL_STM_IF;
    s->u.iff.cond = e;
    s->u.iff.then = t;
    s->u.iff.elsee = f;
    return s;
}

S Hil_Stm_new_do(E e, List_t t, Label_t lf, Label_t le, List_t padding) {
    S s;
    Mem_NEW(s);
    s->kind = HIL_STM_DO;
    s->u.doo.cond = e;
    s->u.doo.body = t;
    s->u.doo.entryLabel = lf;
    s->u.doo.exitLabel = le;
    s->u.doo.padding = padding;
    return s;
}

S Hil_Stm_new_jump(Label_t jump) {
    S s;

    Mem_NEW(s);
    s->kind = HIL_STM_JUMP;
    s->u.jump = jump;
    return s;
}

S Hil_Stm_new_localThrow(Label_t label) {
    S s;

    Mem_NEW(s);
    s->kind = HIL_STM_LOCALTHROW;
    s->u.localThrow = label;
    return s;
}

S Hil_Stm_new_throw() {
    S s;

    Mem_NEW(s);
    s->kind = HIL_STM_THROW;
    return s;
}

S Hil_Stm_new_tryCatch(List_t tryy, List_t catchh, Label_t label, Label_t end) {
    S s;

    Mem_NEW(s);
    s->kind = HIL_STM_TRYCATCH;
    s->u.tryCatch.tryy = tryy;
    s->u.tryCatch.catchh = catchh;
    s->u.tryCatch.label = label;
    s->u.tryCatch.end = end;
    return s;
}

S Hil_Stm_new_return(E e) {
    S s;
    Mem_NEW(s);
    s->kind = HIL_STM_RETURN;
    s->u.returnn = e;
    return s;
}

static int current = 0;

static void indent() {
    current += 2;
}

static void unindent() {
    current -= 2;
}

static void spaces(File_t file) {
    int i = current;
    while (i) {
        fprintf(file, " ");
        i--;
    }
}

File_t Hil_Stm_print(File_t file, S s) {
    assert(s);
    switch (s->kind) {
        case HIL_STM_ASSIGN:
            spaces(file);
            Hil_Lval_print(file, s->u.assign.lval);
            fprintf(file, " = ");
            Hil_Exp_print(file, s->u.assign.exp);
            fprintf(file, ";");
            break;
        case HIL_STM_EXP:
            spaces(file);
            Hil_Exp_print(file, s->u.exp);
            fprintf(file, ";");
            break;
        case HIL_STM_IF:
            spaces(file);
            fprintf(file, "if (");
            Hil_Exp_print(file, s->u.iff.cond);
            fprintf(file, ")\n");
            indent();
            List_foldl(s->u.iff.then, file, (Poly_tyFold) Hil_Stm_print);
            unindent();
            spaces(file);
            fprintf(file, "else\n");
            indent();
            List_foldl(s->u.iff.elsee, file, (Poly_tyFold) Hil_Stm_print);
            unindent();
            break;
        case HIL_STM_DO:
            spaces(file);
            fprintf(file, "do {\n");
            indent();
            List_foldl(s->u.doo.body, file, (Poly_tyFold) Hil_Stm_print);
            unindent();
            spaces(file);
            fprintf(file, "%s:\n", Label_toString(s->u.doo.entryLabel));
            indent();
            List_foldl(s->u.doo.padding, file, (Poly_tyFold) Hil_Stm_print);
            unindent();
            spaces(file);
            fprintf(file, "} while (");
            Hil_Exp_print(file, s->u.doo.cond);
            fprintf(file, ", ");
            fprintf(file, "Lentry=");
            fprintf(file, "%s", Label_toString(s->u.doo.entryLabel));
            fprintf(file, " Lexit=");
            fprintf(file, "%s", Label_toString(s->u.doo.exitLabel));

            fprintf(file, ")");
            break;
        case HIL_STM_JUMP:
            spaces(file);
            fprintf(file, "%s", "jump ");
            fprintf(file, "%s", Label_toString(s->u.jump));
            break;
        case HIL_STM_LOCALTHROW:
            spaces(file);
            fprintf(file, "%s", "localThrow: ");
            fprintf(file, "%s", Label_toString(s->u.localThrow));
            break;
        case HIL_STM_THROW:
            spaces(file);
            fprintf(file, "%s", "throw");
            break;
        case HIL_STM_TRYCATCH: {
            spaces(file);
            fprintf(file, "%s", "try{");
            List_foldl(s->u.tryCatch.tryy, file, (Poly_tyFold) Hil_Stm_print);
            fprintf(file, "%s", "}\n");
            spaces(file);
            fprintf(file, "%s", "catch{");
            fprintf(file, "%s", Label_toString(s->u.tryCatch.label));
            fprintf(file, "%s", "\n");
            List_foldl(s->u.tryCatch.catchh, file, (Poly_tyFold) Hil_Stm_print);
            spaces(file);
            fprintf(file, "%s\n", "}");
            break;
        }
        case HIL_STM_RETURN:
            spaces(file);
            fprintf(file, "return ");
            Hil_Exp_print(file, s->u.returnn);
            fprintf(file, ";");
            break;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible();
            break;
    }
    fprintf(file, "\n");
    return file;
}

//////////////////////////////////////////////////////
/* function */
F Hil_Fun_new(Atype_t type, Id_t name, List_t args, List_t decs, List_t stms) {
    F f;
    Mem_NEW(f);
    f->type = type;
    f->name = name;
    f->args = args;
    f->decs = decs;
    f->stms = stms;
    return f;
}

File_t Hil_Fun_print(File_t file, F f) {
    assert(f);
    fprintf(file, "%s ", Atype_toString(f->type));
    fprintf(file, "%s", Id_toString(f->name));
    fprintf(file, "(");
    List_foldl(f->args, file, (Poly_tyFold) Dec_printAsArg);
    fprintf(file, ")\n{\n");
    indent();
    List_foldl(f->decs, file, (Poly_tyFold) Dec_printAsLocal);
    fprintf(file, "\n");
    List_foldl(f->stms, file, (Poly_tyFold) Hil_Stm_print);
    unindent();
    fprintf(file, "}\n\n");
    return file;
}

//////////////////////////////////////////////////////
// Program
P Hil_Prog_new(List_t classes, List_t funcs) {
    P p;
    Mem_NEW(p);
    p->classes = classes;
    p->funcs = funcs;
    return p;
}

File_t Hil_Prog_print(File_t file, P x) {
    assert(file);
    assert(x);

    List_foldl(x->classes, file, (Poly_tyFold) Class_print);
    fprintf(file, "\n");
    List_foldl(x->funcs, file, (Poly_tyFold) Hil_Fun_print);
    return file;
}

#undef E
#undef F
#undef L
#undef P
#undef S
