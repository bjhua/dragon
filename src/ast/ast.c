#include "ast.h"
#include "../control/control.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include "../lib/unused.h"
#include <assert.h>

#define Id_t AstId_t

#define B Ast_Block_t
#define C Ast_Class_t
#define D Ast_Dec_t
#define Ex Ast_Exp_t
#define F Ast_Fun_t
#define L Ast_Lval_t
#define P Ast_Prog_t
#define S Ast_Stm_t
#define T Ast_Type_t

static void spaces(File_t);

//=====================================================//
/* left-value */
L Ast_Lval_new_var(Id_t var, Type_t ty, Region_t r) {
    L l;
    Mem_NEW(l);
    l->kind = AST_LVAL_VAR;
    l->u.var = var;
    l->ty = ty;
    l->region = r;
    return l;
}

L Ast_Lval_new_dot(L lval, Id_t var, Type_t ty,
                   Region_t r) {
    L l;
    Mem_NEW(l);
    l->kind = AST_LVAL_DOT;
    l->u.dot.lval = lval;
    l->u.dot.var = var;
    l->ty = ty;
    l->region = r;
    return l;
}

L Ast_Lval_new_array(L lval, E e, Type_t ty, Region_t r) {
    L l;
    Mem_NEW(l);
    l->kind = AST_LVAL_ARRAY;
    l->u.array.lval = lval;
    l->u.array.exp = e;
    l->ty = ty;
    l->region = r;
    return l;
}

File_t Ast_Lval_print(File_t file, L l) {
    assert(l);

    switch (l->kind) {
        case AST_LVAL_VAR:
            fprintf(file, "%s", AstId_toString(l->u.var));
            break;
        case AST_LVAL_DOT:
            Ast_Lval_print(file, l->u.dot.lval);
            fprintf(file, "%s", ".");
            fprintf(file, "%s", AstId_toString(l->u.dot.var));
            break;
        case AST_LVAL_ARRAY:
            Ast_Lval_print(file, l->u.array.lval);
            fprintf(file, "%s", "[");
            Ast_Exp_print(file, l->u.array.exp);
            fprintf(file, "%s", "]");
            break;
        default:
            Error_impossible();
            break;
    }
    assert(l);
    if (Control_showType) {
        if (l->ty)
            fprintf(file, "@: %s @", Type_toString(l->ty));
        else
            fprintf(file, "%s", "@NO_TYPE@");
    }
    return file;
}

//=====================================================//
/* expression */
E Ast_Exp_new_assign(E left, E right, Type_t ty,
                     Region_t r) {
    E e;

    Mem_NEW(e);
    e->kind = AST_EXP_ASSIGN;
    e->u.bop.left = left;
    e->u.bop.right = right;
    e->ty = ty;
    e->region = r;
    return e;
}

E Ast_Exp_new_bop(String_t bop,
                  E left,
                  E right,
                  Type_t ty,
                  Region_t r) {
    E e;
    Mem_NEW(e);
    e->u.bop.bop = bop;
    e->u.bop.left = left;
    e->u.bop.right = right;
    e->ty = ty;
    e->region = r;
    return e;
}

E Ast_Exp_new_unary(Ast_Exp_Kind_t kind, E x, Type_t ty, Region_t r) {
    E e;
    Mem_NEW(e);
    e->kind = kind;
    e->u.unary.e = x;
    e->ty = ty;
    e->region = r;
    return e;
}


E Ast_Exp_new_null(void) {
    E e;
    Mem_NEW(e);
    e->kind = AST_EXP_NULL;
    return e;
}

E Ast_Exp_new_intlit(String_t s) {
    E e;
    Mem_NEW(e);
    e->kind = AST_EXP_INTLIT;
    e->u.intlit = strtol(s, 0, 10);
    return e;
}

E Ast_Exp_new_stringlit(String_t s) {
    E e;
    Mem_NEW(e);
    e->kind = AST_EXP_STRINGLIT;
    e->u.stringlit = s;
    return e;
}

E Ast_Exp_new_newArray(T t, Type_t ty, E size) {
    E e;

    Mem_NEW(e);
    e->kind = AST_EXP_NEW_ARRAY;
    e->u.newArray.type = t;
    e->u.newArray.size = size;
    e->ty = ty;
    return e;
}

E Ast_Exp_new_newClass(Id_t id, List_t args, Type_t ty) {
    E e;

    Mem_NEW(e);
    e->kind = AST_EXP_NEW_CLASS;
    e->u.newClass.name = id;
    e->u.newClass.args = args;
    e->ty = ty;
    return e;
}

E Ast_Exp_new_lval(L lval, Type_t ty) {
    E e;

    Mem_NEW(e);
    e->kind = AST_EXP_LVAL;
    e->u.lval = lval;
    e->ty = ty;
    return e;
}

E Ast_Exp_new_call(Id_t f, List_t args, Type_t ty) {
    E e;

    Mem_NEW(e);
    e->kind = AST_EXP_CALL;
    e->u.call.f = f;
    e->u.call.args = args;
    e->ty = ty;
    return e;
}

File_t Ast_Exp_print(File_t file, E e) {
    assert(e);

    switch (e->kind) {
        case AST_EXP_ASSIGN:
            fprintf(file, "%s", "(");
            Ast_Exp_print(file, e->u.assign.left);
            fprintf(file, "%s", " = ");
            Ast_Exp_print(file, e->u.assign.right);
            fprintf(file, "%s", ")");
            break;
        case AST_EXP_BOP:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", e->u.bop.bop);
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_OR:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " || ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_AND:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " && ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_EQ:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " == ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_NE:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " != ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_LT:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " < ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_LE:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " <= ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_GT:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " > ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_GE:
            Ast_Exp_print(file, e->u.bop.left);
            fprintf(file, "%s", " >= ");
            Ast_Exp_print(file, e->u.bop.right);
            break;
        case AST_EXP_NOT:
            fprintf(file, "%s", " ! ");
            Ast_Exp_print(file, e->u.unary.e);
            break;
        case AST_EXP_NEGATIVE:
            fprintf(file, "%s", " - ");
            Ast_Exp_print(file, e->u.unary.e);
            break;
        case AST_EXP_NULL:
            fprintf(file, "%s", " null ");
            break;
        case AST_EXP_INTLIT:
            fprintf(file, "%ld", e->u.intlit);
            break;
        case AST_EXP_STRINGLIT:
            fprintf(file, "%s", "\"");
            fprintf(file, "%s", e->u.stringlit);
            fprintf(file, "%s", "\"");
            break;
        case AST_EXP_NEW_CLASS: {
            List_t p = List_getFirst(e->u.newClass.args);

            fprintf(file, "%s", "new ");
            fprintf(file, "%s", AstId_toString(e->u.newClass.name));
            fprintf(file, "%s", "(");
            while (p) {
                E f = (E) p->data;
                Ast_Exp_print(file, f);
                if (p->next)
                    fprintf(file, "%s", ", ");
                p = p->next;
            }
            fprintf(file, "%s", ")");
            break;
        }
        case AST_EXP_NEW_ARRAY:
            fprintf(file, "%s", "new ");
            Ast_Type_print(file, e->u.newArray.type);
            fprintf(file, "%s", "[");
            Ast_Exp_print(file, e->u.newArray.size);
            fprintf(file, "%s", "]");
            break;
        case AST_EXP_CALL:
            fprintf(file, "%s", AstId_toString(e->u.call.f));
            fprintf(file, "%s", "(");
            List_foldl(e->u.call.args, file, (Poly_tyFold) Ast_Exp_print);
            fprintf(file, "%s", ")");
            break;
        case AST_EXP_LVAL:
            Ast_Lval_print(file, e->u.lval);
            break;
        default:
            fprintf(stderr, "%d", e->kind);
            Error_impossible();
            break;
    }
    assert(e);
    if (Control_showType)
        if (e->ty)
            fprintf(file, "@ %s @", Type_toString(e->ty));
        else
            fprintf(file, "%s", "@NO_TYPE@");
    else
        ;
    return file;
}

////////////////////////////////////////////////
// statements
S Ast_Stm_new_exp(E e) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_EXP;
    s->u.exp = e;
    s->region = 0;
    return s;
}

S Ast_Stm_new_if(E e, S t, S f, Region_t r) {
    S s;
    Mem_NEW(s);
    s->kind = AST_STM_IF;
    s->u.iff.cond = e;
    s->u.iff.then = t;
    s->u.iff.elsee = f;
    s->region = r;
    return s;
}

S Ast_Stm_new_while(E e, S t, Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_WHILE;
    s->u.whilee.cond = e;
    s->u.whilee.body = t;
    s->region = r;
    return s;
}

S Ast_Stm_new_do(E e, S t, Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_DO;
    s->u.doo.cond = e;
    s->u.doo.body = t;
    s->region = r;
    return s;
}

S Ast_Stm_new_for(E header, E e, E tail,
                  S body, Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_FOR;
    s->u.forr.header = header;
    s->u.forr.cond = e;
    s->u.forr.tail = tail;
    s->u.forr.body = body;
    s->region = r;
    return s;
}

S Ast_Stm_new_break(Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_BREAK;
    s->region = r;
    return s;
}

S Ast_Stm_new_continue(Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_CONTINUE;
    s->region = r;
    return s;
}

S Ast_Stm_new_throw(Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_THROW;
    s->region = r;
    return s;
}

S Ast_Stm_new_tryCatch(S tryy, S catchh, Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_TRYCATCH;
    s->u.trycatch.tryy = tryy;
    s->u.trycatch.catchh = catchh;
    s->region = r;
    return s;
}

S Ast_Stm_new_return(E e, Region_t r) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_RETURN;
    s->u.returnn = e;
    s->region = r;
    return s;
}

S Ast_Stm_new_block(B b) {
    S s;

    Mem_NEW(s);
    s->kind = AST_STM_BLOCK;
    s->u.block = b;
    return s;
}

static int current = 0;

static void indent(void) {
    current += 2;
}

static void unindent(void) {
    current -= 2;
}

static void spaces(File_t file) {
    int i = current;
    while (i) {
        fprintf(file, "%s", " ");
        i--;
    }
}

Box_t Ast_Stm_box(S s) {
    assert(s);
    return Box_str("<junk>;");
}

File_t Ast_Stm_print(File_t file, S s) {
    assert(s);
    switch (s->kind) {
        case AST_STM_EXP: {
            spaces(file);
            Ast_Exp_print(file, s->u.exp);
            fprintf(file, "%s", ";");
            break;
        }
        case AST_STM_IF:
            spaces(file);
            fprintf(file, "%s", "if (");
            Ast_Exp_print(file, s->u.iff.cond);
            fprintf(file, "%s", ")\n");
            indent();
            Ast_Stm_print(file, s->u.iff.then);
            unindent();
            if (s->u.iff.elsee) {
                spaces(file);
                fprintf(file, "%s", "else\n");
                indent();
                Ast_Stm_print(file, s->u.iff.elsee);
                unindent();
            }
            break;
        case AST_STM_WHILE:
            spaces(file);
            fprintf(file, "%s", "while (");
            Ast_Exp_print(file, s->u.whilee.cond);
            fprintf(file, "%s", ")\n");
            indent();
            Ast_Stm_print(file, s->u.whilee.body);
            unindent();
            break;
        case AST_STM_DO:
            spaces(file);
            fprintf(file, "%s", "do {\n");
            indent();
            Ast_Stm_print(file, s->u.doo.body);
            unindent();
            spaces(file);
            fprintf(file, "%s", "} while (");
            Ast_Exp_print(file, s->u.doo.cond);
            fprintf(file, "%s", ")");
            break;
        case AST_STM_FOR:
            spaces(file);
            fprintf(file, "%s", "for (");
            Ast_Exp_print(file, s->u.forr.header);
            fprintf(file, "%s", "; ");
            Ast_Exp_print(file, s->u.forr.cond);
            fprintf(file, "%s", "; ");
            Ast_Exp_print(file, s->u.forr.tail);
            fprintf(file, "%s", ")");
            Ast_Stm_print(file, s->u.forr.body);
            break;
        case AST_STM_BREAK:
            spaces(file);
            fprintf(file, "%s", "break;");
            break;
        case AST_STM_CONTINUE:
            spaces(file);
            fprintf(file, "%s", "continue;");
            break;
        case AST_STM_THROW: {
            spaces(file);
            fprintf(file, "%s", "throw");
            break;
        }
        case AST_STM_TRYCATCH:
            spaces(file);
            fprintf(file, "%s", "try ");
            Ast_Stm_print(file, s->u.trycatch.tryy);
            fprintf(file, "%s", "catch ");
            Ast_Stm_print(file, s->u.trycatch.catchh);
            break;
        case AST_STM_RETURN:
            spaces(file);
            fprintf(file, "%s", "return ");
            Ast_Exp_print(file, s->u.returnn);
            fprintf(file, "%s", ";");
            break;
        case AST_STM_BLOCK:
            Ast_Block_print(file, s->u.block);
            break;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible();
            break;
    }
    fprintf(file, "%s", "\n");
    return file;
}

/////////////////////////////////////////////////
/* declarations */
D Ast_Dec_new(T type, Id_t var, E init) {
    D d;

    Mem_NEW(d);
    d->type = type;
    d->var = var;
    d->init = init;
    return d;
}

static Box_t Ast_Dec_box(D d) {
    assert(d);

    return Box_h(Ast_Type_box(d->type),
                 Box_str(AstId_toString(d->var)),
                 (d->init) ? (Box_str(" = <TODO>;")) : Box_str(";"),
                 0);
}

File_t Ast_Dec_print(File_t file, D d) {
    assert(d);
    spaces(file);
    fprintf(file, "%s", Ast_Type_toString(d->type));
    fprintf(file, "%s", " ");
    fprintf(file, "%s", AstId_toString(d->var));
    if (d->init) {
        fprintf(file, "%s", " = ");
        Ast_Exp_print(file, d->init);
    }
    fprintf(file, "%s", ";\n");
    return file;
}

///////////////////////////////////////////////
// Block
B Ast_Block_new(List_t decs, List_t stms) {
    B b;
    Mem_NEW(b);
    b->decs = decs;
    b->stms = stms;
    return b;
}

static Box_t Ast_Block_box(B b) {
    Box_t b1, b2, b3;

    assert(b);
    b1 = Box_str("{");
    b2 = Box_vlist(List_map(b->decs,
                            (Poly_tyId) Ast_Dec_box));
    b3 = Box_vlist(List_map(b->stms,
                            (Poly_tyId) Ast_Stm_box));
    b2 = Box_v(b2, b3, 0);
    b3 = Box_str("}");
    return Box_v(b1, Box_indent(b2, 2), b3, 0);
}

File_t Ast_Block_print(File_t file, B b) {
    assert(b);
    spaces(file);
    fprintf(file, "%s", "{\n");
    indent();
    List_foldl(b->decs, file, (Poly_tyFold) Ast_Dec_print);
    List_foldl(b->stms, file, (Poly_tyFold) Ast_Stm_print);
    unindent();
    spaces(file);
    fprintf(file, "%s", "}");
    return file;
}

////////////////////////////////////////////////
// function
F Ast_Fun_new(T type, Id_t name, List_t args,
              B block, Region_t r) {
    F f;
    Mem_NEW(f);
    f->type = type;
    f->name = name;
    f->args = args;
    f->block = block;
    f->region = r;
    return f;
}

Box_t Ast_Fun_box(F f) {
    Box_t b1, b2;

    assert(f);
    b1 = Box_h(Box_str(Ast_Type_toString(f->type)),
               Box_str(AstId_toString(f->name)),
               Box_str("("),
               Box_hlist(List_map(f->args,
                                  (Poly_tyId) Ast_Dec_box)),
               Box_str(")"),
               0);
    b2 = Ast_Block_box(f->block);
    return Box_v(b1, b2, 0);
}

static File_t Ast_Arguments_print(File_t file, List_t args) {
    Ast_Dec_t dec;

    assert(args);

    args = args->next;
    if (!args)
        return file;
    dec = (Ast_Dec_t) args->data;
    Ast_Type_print(file, dec->type);
    fprintf(file, "%s", " ");
    fprintf(file, "%s", AstId_toString(dec->var));
    args = args->next;
    while (args) {
        fprintf(file, "%s", ", ");
        dec = (Ast_Dec_t) args->data;
        Ast_Type_print(file, dec->type);
        fprintf(file, "%s", " ");
        fprintf(file, "%s", AstId_toString(dec->var));
        args = args->next;
    }
    return file;
}

File_t Ast_Fun_print(File_t file, F f) {
    assert(f);
    Ast_Type_print(file, f->type);
    fprintf(file, "%s", " ");
    fprintf(file, "%s", AstId_toString(f->name));
    fprintf(file, "%s", "(");
    Ast_Arguments_print(file, f->args);
    fprintf(file, "%s", ")\n");
    Ast_Block_print(file, f->block);
    fprintf(file, "%s", "\n\n");
    return file;
}

///////////////////////////////////////////////
// type
T Ast_Type_new_int(void) {
    T t;

    Mem_NEW(t);
    t->kind = AST_TYPE_INT;
    t->isArray = 0;
    return t;
}

T Ast_Type_new_string(void) {
    T t;

    Mem_NEW(t);
    t->kind = AST_TYPE_STRING;
    t->isArray = 0;
    return t;
}

T Ast_Type_new_id(Id_t id) {
    T t;

    Mem_NEW(t);
    t->kind = AST_TYPE_ID;
    t->id = id;
    t->isArray = 0;
    return t;
}

void Ast_Type_setArray(T t) {
    assert(t);

    if (t->isArray)
        Error_impossible();
    assert(t);
    t->isArray = 1;
}

Box_t Ast_Type_box(T x) {
    assert(x);

    Error_bug("TODO");
    //    return 0;
}

File_t Ast_Type_print(File_t file, T x) {
    assert(x);
    switch (x->kind) {
        case AST_TYPE_INT:
            fprintf(file, "%s", "int");
            break;
        case AST_TYPE_STRING:
            fprintf(file, "%s", "string");
            break;
        case AST_TYPE_ID:
            fprintf(file, "%s", AstId_toString(x->id));
            break;
        default:
            Error_impossible();
            return file;
    }
    if (x->isArray)
        fprintf(file, "%s", "[]");
    return file;
}

String_t Ast_Type_toString(T x) {
    String_t s;

    assert(x);
    switch (x->kind) {
        case AST_TYPE_INT:
            s = "int";
            break;
        case AST_TYPE_STRING:
            s = "string";
            break;
        case AST_TYPE_ID:
            s = AstId_toString(x->id);
            break;
        default:
            Error_impossible();
            return 0;
    }
    if (x->isArray)
        s = String_concat(s, "[]", 0);
    return s;
}


////////////////////////////////////////
// class
C Ast_Class_new(Id_t name, List_t decs) {
    C c;

    Mem_NEW(c);
    c->name = name;
    c->fields = decs;
    return c;
}

Box_t Ast_Class_box(C c) {
    // to suppress warnings
    UNUSED(c);
    return 0;
}

File_t Ast_Class_print(File_t file, C c) {
    List_t p;

    assert(c);

    p = List_getFirst(c->fields);
    fprintf(file, "%s", "\nclass ");
    fprintf(file, "%s", AstId_toString(c->name));
    fprintf(file, "%s", "\n{\n");
    while (p) {
        D dec = (D) p->data;
        fprintf(file, "%s", "  ");
        fprintf(file, "%s", Ast_Type_toString(dec->type));
        fprintf(file, "%s", " ");
        fprintf(file, "%s", AstId_toString(dec->var));
        fprintf(file, "%s", ";\n");
        p = p->next;
    }
    fprintf(file, "%s", "}\n");
    return file;
}

/////////////////////////////////////////////
// program
P Ast_Prog_new(List_t classes, List_t funcs) {
    P p;

    Mem_NEW(p);
    p->classes = classes;
    p->funcs = funcs;
    return p;
}

Box_t Ast_Prog_box(P x) {
    List_t boxes =
            List_map(x->classes,
                     (Poly_tyId) Ast_Class_box);
    List_t boxes2 =
            List_map(x->funcs,
                     (Poly_tyId) Ast_Fun_box);
    List_append(boxes, boxes2);
    return Box_vlist(boxes);
}

void Ast_Prog_print(File_t file, P x) {
    assert(x);
    List_foldl(x->classes, file, (Poly_tyFold) Ast_Class_print);
    fprintf(file, "%s", "\n");
    List_foldl(x->funcs, file, (Poly_tyFold) Ast_Fun_print);
    //    return;
}

#undef B
#undef C
#undef D
#undef E
#undef P
#undef T
#undef F
#undef S
#undef L


#undef E

#undef Id_t
