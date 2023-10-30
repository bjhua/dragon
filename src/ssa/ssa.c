#include "../lib/mem.h"
#include "../lib/int.h"
#include "../lib/assert.h"
#include "../lib/set.h"
#include "../lib/hash-set.h"
#include "../lib/tuple.h"
#include "../lib/trace.h"
#include "ssa.h"

#define B Ssa_Block_t
#define F Ssa_Fun_t
#define M Ssa_Mem_t
#define O Ssa_Operand_t
#define P Ssa_Prog_t
#define S Ssa_Stm_t
#define T Ssa_Transfer_t

///////////////////////////////////////////////////////
static String_t dotname = "none";
static int semi = 1;


///////////////////////////////////////////////////////
// to build a closure
struct GlobalUd_t {
    Id_t (*use)(Id_t);

    Id_t (*def)(Id_t);
};

struct GlobalUd_t gud = {0, 0};

/////////////////////////////////////////////////////
static List_t globalSubstPhi = 0;

static Id_t newsubstPhiLookup(Ssa_Stm_PhiArg_t a) {
    List_t s = List_getFirst(globalSubstPhi);

    while (s) {
        Tuple_t t = (Tuple_t) s->data;
        Ssa_Stm_PhiArg_t arg = Tuple_first(t);
        Id_t newid = Tuple_second(t);

        if (Ssa_Stm_PhiArg_equals(a, arg))
            return newid;

        s = s->next;
    }
    return 0;
}

///////////////////////////////////////////////////////
/* operands */
O Ssa_Operand_new_int(int i) {
    O e;

    Mem_NEW (e);
    e->kind = SSA_OP_INT;
    e->u.intlit = i;
    return e;
}

O Ssa_Operand_new_string(String_t str) {
    O e;

    Mem_NEW (e);
    e->kind = SSA_OP_STR;
    e->u.strlit = str;
    return e;
}


O Ssa_Operand_new_id(Id_t id) {
    O e;

    Mem_NEW (e);
    e->kind = SSA_OP_ID;
    e->u.id = id;
    return e;
}

static O Ssa_Operand_renameUse(O o, Id_t (*use)(Id_t)) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return o;
        case SSA_OP_STR:
            return o;
        case SSA_OP_ID:
            return Ssa_Operand_new_id(use(o->u.id));
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

// this function should be combined with the above one.
static O Ssa_Operand_renameUse2Op(O o, O (*f)(Id_t)) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return o;
        case SSA_OP_STR:
            return o;
        case SSA_OP_ID: {
            O new = f(o->u.id);

            if (new)
                return new;
            else
                return o;
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

static void Ssa_Operand_foreachUse(O o, void (*f)(Id_t)) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return;
        case SSA_OP_STR:
            return;
        case SSA_OP_ID:
            return f(o->u.id);
        default:
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

static O (*globalf)(Id_t) = 0;

static O Ssa_Operand_renameUse2OpList(O o) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return o;
        case SSA_OP_STR:
            return o;
        case SSA_OP_ID: {
            O new = globalf(o->u.id);

            if (new)
                return new;
            else
                return o;
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

// Only useful when gud.use is effective.
static O Ssa_Operand_renameUseGud(O o) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return o;
        case SSA_OP_STR:
            return o;
        case SSA_OP_ID:
            return Ssa_Operand_new_id(gud.use(o->u.id));
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

int Ssa_Operand_isConst(O o) {
    Assert_ASSERT(o);

    switch (o->kind) {
        case SSA_OP_INT:
            return 1;
        case SSA_OP_STR:
            return 1;
        case SSA_OP_ID:
            return 0;
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

int Ssa_Operand_isSameConst(List_t os) {
    Ssa_Operand_t first;

    Assert_ASSERT(os);

    if (List_isEmpty(os))
        Error_impossible ();

    os = List_getFirst(os);
    first = os->data;
    if (!Ssa_Operand_isConst(first))
        return 0;

    os = os->next;
    while (os) {
        Ssa_Operand_t current = os->data;

        if (!Ssa_Operand_isConst(current))
            return 0;
        if (!Ssa_Operand_equals(first, current))
            return 0;
        os = os->next;
    }
    return 1;
}

int Ssa_Operand_equals(O o1, O o2) {
    Assert_ASSERT(o1);
    Assert_ASSERT(o2);

    if (o1->kind != o2->kind)
        return 0;

    switch (o1->kind) {
        case SSA_OP_INT:
            return o1->u.intlit == o2->u.intlit;
        case SSA_OP_STR:
            // this tends to be too conservative.
            return o1->u.strlit == o2->u.strlit;
        case SSA_OP_ID:
            return Id_equals(o1->u.id, o2->u.id);
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

File_t Ssa_Operand_print(File_t file, O o) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            fprintf(file, "%s", Int_toString(o->u.intlit));
            break;
        case SSA_OP_STR:
            fprintf(file, "%s", "\"");
            fprintf(file, "%s", o->u.strlit);
            fprintf(file, "%s", "\"");
            break;
        case SSA_OP_ID:
            fprintf(file, "%s", Id_toString(o->u.id));
            break;
        default:
            Error_impossible ();
            break;
    }
    return file;
}

String_t Ssa_Operand_toString(O o) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case SSA_OP_INT:
            return Int_toString(o->u.intlit);
        case SSA_OP_STR:
            return o->u.strlit;
        case SSA_OP_ID:
            return Id_toString(o->u.id);
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

///////////////////////////////////////////////////////
// memory
M Ssa_Mem_new_array(Id_t name, O index) {
    M m;

    Mem_NEW(m);
    m->kind = SSA_MEM_ARRAY;
    m->u.array.name = name;
    m->u.array.index = index;
    return m;
}

M Ssa_Mem_new_class(Id_t name, Id_t field) {
    M m;

    Mem_NEW(m);
    m->kind = SSA_MEM_CLASS;
    m->u.class.name = name;
    m->u.class.field = field;
    return m;
}

M Ssa_Mem_renameUse(M m, Id_t(*use)(Id_t)) {
    Assert_ASSERT(m);
    switch (m->kind) {
        case SSA_MEM_ARRAY:
            return Ssa_Mem_new_array
                    (use(m->u.array.name), Ssa_Operand_renameUse(m->u.array.index, use));
        case SSA_MEM_CLASS:
            return Ssa_Mem_new_class
                    (use(m->u.class.name), m->u.class.field);
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

void Ssa_Mem_foreachUse(M m, void(*f)(Id_t)) {
    Assert_ASSERT(m);
    switch (m->kind) {
        case SSA_MEM_ARRAY:
            f(m->u.array.name);
            Ssa_Operand_foreachUse(m->u.array.index, f);
            return;
        case SSA_MEM_CLASS:
            f(m->u.class.name);
            return;
        default:
            fprintf(stderr, "%d", m->kind);
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

M Ssa_Mem_renameUse2Op(M m, O(*f)(Id_t)) {
    Assert_ASSERT(m);
    switch (m->kind) {
        case SSA_MEM_ARRAY:
            return Ssa_Mem_new_array
                    // array name can not be const!
                    (m->u.array.name, Ssa_Operand_renameUse2Op(m->u.array.index, f));
        case SSA_MEM_CLASS:
            return Ssa_Mem_new_class
                    // class name can not be const!
                    (m->u.class.name, m->u.class.field);
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

File_t Ssa_Mem_print(File_t file, M m) {
    Assert_ASSERT(m);
    switch (m->kind) {
        case SSA_MEM_ARRAY:
            fprintf(file, "%s", Id_toString(m->u.array.name));
            fprintf(file, "%s", "[");
            Ssa_Operand_print(file, m->u.array.index);
            fprintf(file, "%s", "]");
            break;
        case SSA_MEM_CLASS:
            fprintf(file, "%s", Id_toString(m->u.class.name));
            fprintf(file, "%s", ".");
            fprintf(file, "%s", Id_toString(m->u.class.field));
            break;
        default:
            Error_impossible ();
            return 0;
    }
    return file;
}

/////////////////////////////////////////////////////
// statements
// phi argument
Ssa_Stm_PhiArg_t Ssa_Stm_PhiArg_new(O arg, B pred) {
    Ssa_Stm_PhiArg_t a;

    Mem_NEW(a);
    a->arg = arg;
    a->pred = pred;
    return a;
}

int Ssa_Stm_PhiArg_equals(Ssa_Stm_PhiArg_t a1, Ssa_Stm_PhiArg_t a2) {
    return Ssa_Operand_equals(a1->arg, a2->arg)
           && Ssa_Block_equals(a1->pred, a2->pred);
}

void Ssa_Stm_PhiArg_foreachUse(Ssa_Stm_PhiArg_t a, void (*f)(Id_t)) {
    Ssa_Operand_foreachUse(a->arg, f);
    return;
}

int Ssa_Stm_PhiArg_isSameConst(List_t l) {
    Ssa_Stm_PhiArg_t firstArg;

    if (List_isEmpty(l))
        Error_impossible ();

    l = l->next;
    firstArg = l->data;
    if (!Ssa_Operand_isConst(firstArg->arg))
        return 0;
    l = l->next;
    while (l) {
        Ssa_Stm_PhiArg_t current = l->data;

        if (!Ssa_Operand_isConst(current->arg))
            return 0;
        if (!Ssa_Operand_equals(current->arg, firstArg->arg))
            return 0;
        l = l->next;
    }
    return 1;
}

Ssa_Stm_PhiArg_t Ssa_Stm_PhiArg_renameUse2Op
        (Ssa_Stm_PhiArg_t arg) {
    return Ssa_Stm_PhiArg_new(Ssa_Operand_renameUse2Op
                                      (arg->arg, globalf), arg->pred);
}

File_t Ssa_Stm_PhiArg_print(File_t file, Ssa_Stm_PhiArg_t arg) {
    Assert_ASSERT(arg);

    Ssa_Operand_print(file, arg->arg);
    fprintf(file, "%s", " (");
    fprintf(file, "%s", Label_toString(arg->pred->label));
    fprintf(file, "%s", "), ");
    return file;
}

String_t Ssa_Stm_PhiArg_toString(Ssa_Stm_PhiArg_t arg) {
    Assert_ASSERT(arg);

    return String_concat(Ssa_Operand_toString(arg->arg), " (", Label_toString(arg->pred->label), "), ", 0);
}

/*********************************************************/
// statement
S Ssa_Stm_new_move(Id_t dest, O src) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_MOVE;
    s->u.move.dest = dest;
    s->u.move.src = src;
    return s;
}

S Ssa_Stm_new_bop(Id_t dest, O left, Operator_t opr,
                  O right) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_BOP;
    s->u.bop.dest = dest;
    s->u.bop.left = left;
    s->u.bop.op = opr;
    s->u.bop.right = right;
    return s;
}

S Ssa_Stm_new_uop(Id_t dest, Operator_t opr, O src) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_UOP;
    s->u.uop.dest = dest;
    s->u.uop.op = opr;
    s->u.uop.src = src;
    return s;
}


S Ssa_Stm_new_store(M m, O src) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_STORE;
    s->u.store.m = m;
    s->u.store.src = src;
    return s;
}

S Ssa_Stm_new_load(Id_t dest, M m) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_LOAD;
    s->u.load.dest = dest;
    s->u.load.m = m;
    return s;
}

S Ssa_Stm_new_newClass(Id_t dest, Id_t cname) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_NEW_CLASS;
    s->u.newClass.dest = dest;
    s->u.newClass.cname = cname;
    return s;
}

S Ssa_Stm_new_newArray(Id_t dest, Atype_t type, O size) {
    S s;
    Mem_NEW (s);
    s->kind = SSA_STM_NEW_ARRAY;
    s->u.newArray.dest = dest;
    s->u.newArray.ty = type;
    s->u.newArray.size = size;
    return s;
}

S Ssa_Stm_new_try(Label_t label) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_TRY;
    s->u.try = label;
    return s;
}

S Ssa_Stm_new_try_end(Label_t tryEnd) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_TRY_END;
    s->u.tryEnd = tryEnd;
    return s;
}

// args: List<Arg>
S Ssa_Stm_new_phi(Id_t dest, List_t args) {
    S s;

    Mem_NEW (s);
    s->kind = SSA_STM_PHI;
    s->u.phi.dest = dest;
    s->u.phi.args = args;
    return s;
}

// preds: List<B>
S Ssa_Stm_new_phi_predsBlock(Id_t dest, List_t preds) {
    S s;
    List_t args = List_new();
    preds = List_getFirst(preds);

    Mem_NEW (s);
    s->kind = SSA_STM_PHI;
    s->u.phi.dest = dest;
    while (preds) {
        B b = (B) preds->data;
        Ssa_Stm_PhiArg_t arg
                = Ssa_Stm_PhiArg_new(Ssa_Operand_new_id(dest), b);
        List_insertLast(args, arg);
        preds = preds->next;
    }
    s->u.phi.args = args;
    return s;
}

static Set_t Ssa_Stm_getDefIds(S s) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case SSA_STM_MOVE:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.move.dest);
        case SSA_STM_BOP:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.bop.dest);
        case SSA_STM_UOP:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.uop.dest);

        case SSA_STM_STORE:
            return Set_new((Poly_tyEquals) Id_equals);
        case SSA_STM_LOAD:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.load.dest);
        case SSA_STM_NEW_CLASS:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.newClass.dest);
        case SSA_STM_NEW_ARRAY:
            return Set_singleton((Poly_tyEquals) Id_equals, s->u.newArray.dest);
        case SSA_STM_TRY:
        case SSA_STM_TRY_END:
            return Set_new((Poly_tyEquals) Id_equals);
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

void Ssa_Stm_foreachUse(S s, void (*f)(Id_t)) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case SSA_STM_MOVE:
            Ssa_Operand_foreachUse(s->u.move.src, f);
            return;
        case SSA_STM_BOP:
            Ssa_Operand_foreachUse(s->u.bop.left, f);
            Ssa_Operand_foreachUse(s->u.bop.right, f);
            return;
        case SSA_STM_UOP:
            Ssa_Operand_foreachUse(s->u.uop.src, f);
            return;

        case SSA_STM_STORE:
            Ssa_Mem_foreachUse(s->u.store.m, f);
            Ssa_Operand_foreachUse(s->u.store.src, f);
            return;
        case SSA_STM_LOAD:
            Ssa_Mem_foreachUse(s->u.load.m, f);
            return;
        case SSA_STM_NEW_CLASS:
            return;
        case SSA_STM_NEW_ARRAY:
            Ssa_Operand_foreachUse(s->u.newArray.size, f);
            return;
            // only rename def
        case SSA_STM_PHI: {
            List_t l = List_getFirst(s->u.phi.args);
            while (l) {
                Ssa_Stm_PhiArg_foreachUse(l->data, f);
                l = l->next;
            }
            return;
        }
        case SSA_STM_TRY:
        case SSA_STM_TRY_END:
            return;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

S Ssa_Stm_renameUseDefNoPhiUse(S s) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case SSA_STM_MOVE: {
            S new;

            printf("move starts\n");
            new =
                    Ssa_Stm_new_move
                            (gud.def(s->u.move.dest), Ssa_Operand_renameUse(s->u.move.src, gud.use));
            printf("the id is: %s\n", Id_toString(s->u.move.dest));

            printf("move finished\n");
            return new;
        }
        case SSA_STM_BOP:
            return Ssa_Stm_new_bop
                    (gud.def(s->u.bop.dest), Ssa_Operand_renameUse(s->u.bop.left, gud.use), s->u.bop.op,
                     Ssa_Operand_renameUse(s->u.bop.right, gud.use));
        case SSA_STM_UOP:
            return Ssa_Stm_new_uop
                    (gud.def(s->u.uop.dest), s->u.uop.op, Ssa_Operand_renameUse(s->u.uop.src, gud.use));
        case SSA_STM_STORE:
            return Ssa_Stm_new_store
                    (Ssa_Mem_renameUse(s->u.store.m, gud.use), Ssa_Operand_renameUse(s->u.store.src, gud.use));
        case SSA_STM_LOAD:
            return Ssa_Stm_new_load
                    (gud.def(s->u.load.dest), Ssa_Mem_renameUse(s->u.load.m, gud.use));
        case SSA_STM_NEW_CLASS:
            return Ssa_Stm_new_newClass
                    (gud.def(s->u.newClass.dest), s->u.newClass.cname);
        case SSA_STM_NEW_ARRAY:
            return Ssa_Stm_new_newArray
                    (gud.def(s->u.newArray.dest), s->u.newArray.ty, Ssa_Operand_renameUse(s->u.newArray.size, gud.use));
            // only rename def
        case SSA_STM_PHI:
            return Ssa_Stm_new_phi
                    (gud.def(s->u.phi.dest), s->u.phi.args);
        case SSA_STM_TRY:
        case SSA_STM_TRY_END:
            return s;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

// use in const propagation. can be combined with the 
// "renameUseDefNoPhiUse"?
S Ssa_Stm_renameUse2Op(S s, O (*f)(Id_t)) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case SSA_STM_MOVE: {
            // this case is specail in that: if this is the
            // the candidate, delete this stm.
            if (f(s->u.move.dest))
                return 0;

            return
                    Ssa_Stm_new_move
                            (s->u.move.dest, Ssa_Operand_renameUse2Op(s->u.move.src, f));
        }
        case SSA_STM_BOP: {
            if (f(s->u.bop.dest))
                return 0;

            return Ssa_Stm_new_bop
                    (s->u.bop.dest, Ssa_Operand_renameUse2Op(s->u.bop.left, f), s->u.bop.op,
                     Ssa_Operand_renameUse2Op(s->u.bop.right, f));
        }
        case SSA_STM_UOP: {
            if (f(s->u.uop.dest))
                return 0;

            return Ssa_Stm_new_uop
                    (s->u.uop.dest, s->u.uop.op, Ssa_Operand_renameUse2Op(s->u.uop.src, f));
        }

        case SSA_STM_STORE:
            return Ssa_Stm_new_store
                    (Ssa_Mem_renameUse2Op(s->u.store.m, f), Ssa_Operand_renameUse2Op(s->u.store.src, f));
        case SSA_STM_LOAD:
            return Ssa_Stm_new_load
                    (s->u.load.dest, Ssa_Mem_renameUse2Op(s->u.load.m, f));
        case SSA_STM_NEW_CLASS:
            return Ssa_Stm_new_newClass
                    (s->u.newClass.dest, s->u.newClass.cname);
        case SSA_STM_NEW_ARRAY:
            return Ssa_Stm_new_newArray
                    (s->u.newArray.dest, s->u.newArray.ty, Ssa_Operand_renameUse2Op(s->u.newArray.size, f));
            // this case is special
        case SSA_STM_PHI: {
            O src;

            globalf = f;
            if ((src = f(s->u.phi.dest)))
                return Ssa_Stm_new_move(s->u.phi.dest, src);

            return Ssa_Stm_new_phi
                    (s->u.phi.dest, List_map(s->u.phi.args, (Poly_tyId) Ssa_Stm_PhiArg_renameUse2Op));
        }
        case SSA_STM_TRY:
        case SSA_STM_TRY_END:
            return s;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

S Ssa_Stm_renamePhiArgs(S s) {
    List_t newArgs = List_new();
    List_t oldArgs;
    Assert_ASSERT(s);
    if (s->kind != SSA_STM_PHI)
        return s;

    oldArgs = List_getFirst(s->u.phi.args);
    while (oldArgs) {
        Ssa_Stm_PhiArg_t a = (Ssa_Stm_PhiArg_t) oldArgs->data;
        Id_t newid;

        if (a->arg->kind != SSA_OP_ID)
            Error_impossible ();

        newid = newsubstPhiLookup(a);
        if (!newid) {
            Ssa_Operand_print(stdout, a->arg);
            Error_impossible ();
        }

        List_insertLast(newArgs, Ssa_Stm_PhiArg_new
                (Ssa_Operand_new_id(newid), a->pred));

        oldArgs = oldArgs->next;
    }

    return Ssa_Stm_new_phi(s->u.phi.dest, newArgs);
}

static void Ssa_Stm_foreachDef(S s, void (*f)(Id_t)) {
    Assert_ASSERT(s);
    switch (s->kind) {
        case SSA_STM_MOVE:
            f(s->u.move.dest);
            return;
        case SSA_STM_BOP:
            f(s->u.bop.dest);
            return;
        case SSA_STM_UOP:
            f(s->u.uop.dest);
            return;
        case SSA_STM_STORE:
            return;
        case SSA_STM_LOAD:
            f(s->u.load.dest);
            return;
        case SSA_STM_NEW_CLASS:
            f(s->u.newClass.dest);
            return;
        case SSA_STM_NEW_ARRAY:
            f(s->u.newArray.dest);
            // only rename def
        case SSA_STM_PHI:
            f(s->u.phi.dest);
            return;
        case SSA_STM_TRY:
            return;
        case SSA_STM_TRY_END:
            return;
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

static void spacetab(File_t file) {
    fprintf(file, "\t");
}

File_t Ssa_Stm_print(File_t file, S s) {
    Assert_ASSERT(s);
    spacetab(file);
    switch (s->kind) {
        case SSA_STM_MOVE:
            fprintf(file, "%s", Id_toString(s->u.move.dest));
            fprintf(file, " = ");
            Ssa_Operand_print(file, s->u.move.src);
            break;
        case SSA_STM_BOP:
            fprintf(file, "%s", Id_toString(s->u.bop.dest));
            fprintf(file, " = ");
            Ssa_Operand_print(file, s->u.bop.left);
            fprintf(file, "%s", Operator_toString(s->u.bop.op));
            Ssa_Operand_print(file, s->u.bop.right);
            break;
        case SSA_STM_UOP:
            fprintf(file, "%s", Id_toString(s->u.uop.dest));
            fprintf(file, " = ");
            fprintf(file, "%s", Operator_toString(s->u.uop.op));
            Ssa_Operand_print(file, s->u.uop.src);
            break;

        case SSA_STM_STORE:
            Ssa_Mem_print(file, s->u.store.m);
            fprintf(file, "%s", " = ");
            Ssa_Operand_print(file, s->u.store.src);
            break;
        case SSA_STM_LOAD:
            fprintf(file, "%s", Id_toString(s->u.load.dest));
            fprintf(file, "%s", " = ");
            Ssa_Mem_print(file, s->u.load.m);
            break;
        case SSA_STM_NEW_CLASS: {
            fprintf(file, "%s", Id_toString(s->u.newClass.dest));
            fprintf(file, "%s", " = new ");
            fprintf(file, "%s", Id_toString(s->u.newClass.cname));
            fprintf(file, "%s", " ()");
            break;
        }
        case SSA_STM_NEW_ARRAY: {
            fprintf(file, "%s", Id_toString(s->u.newArray.dest));
            fprintf(file, "%s", " = new ");
            fprintf(file, "%s", Atype_toString(s->u.newArray.ty));
            fprintf(file, "%s", "[");
            Ssa_Operand_print(file, s->u.newArray.size);
            fprintf(file, "%s", "]");
            break;
        }
        case SSA_STM_TRY: {
            fprintf(file, "try (%s)", Label_toString(s->u.try));
            break;
        }
        case SSA_STM_TRY_END: {
            fprintf(file, "try_end(%s)", Label_toString(s->u.tryEnd));
            break;
        }
        case SSA_STM_PHI: {
            fprintf(file, "%s", Id_toString(s->u.phi.dest));
            fprintf(file, "%s", " = PHI");
            fprintf(file, "(");
            List_foldl(s->u.phi.args, file, (Poly_tyFold) Ssa_Stm_PhiArg_print);
            fprintf(file, "%s", ")");
            break;
        }
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            break;
    }
    if (semi)
        fprintf(file, ";\n");
    else
        fprintf(file, ";\\n");
    return file;
}

///////////////////////////////////////////////////////
/* transfer */
T Ssa_Transfer_new_if(O cond, Label_t tt, Label_t f) {
    T t;

    Mem_NEW(t);
    t->kind = SSA_TRANS_IF;
    t->u.iff.cond = cond;
    t->u.iff.truee = tt;
    t->u.iff.falsee = f;
    return t;
}

T Ssa_Transfer_new_jump(Label_t l) {
    T t;

    Mem_NEW(t);
    t->kind = SSA_TRANS_JUMP;
    t->u.jump = l;
    return t;
}

T Ssa_Transfer_new_return(O r) {
    T t;

    Mem_NEW(t);
    t->kind = SSA_TRANS_RETURN;
    t->u.ret = r;
    return t;
}

T Ssa_Transfer_new_throw() {
    T t;

    Mem_NEW(t);
    t->kind = SSA_TRANS_THROW;
    return t;
}

T Ssa_Transfer_new_call(Id_t dest, Id_t f, List_t args, Label_t leave, Label_t normal) {
    T t;

    Mem_NEW (t);
    t->kind = SSA_TRANS_CALL;
    t->u.call.dest = dest;
    t->u.call.name = f;
    t->u.call.args = args;
    t->u.call.leave = leave;
    t->u.call.normal = normal;
    return t;
}



// rename labels only for non-zero label arguments.
T Ssa_Transfer_renameLabels_if(T x, Label_t t, Label_t f) {
    Assert_ASSERT(x);
    return Ssa_Transfer_new_if
            (x->u.iff.cond, t ? t : x->u.iff.truee, f ? f : x->u.iff.falsee);
}

// rename labels only for non-zero label arguments.
T Ssa_Transfer_renameLabels_jump(T x, Label_t l) {
    Assert_ASSERT(x);
    return Ssa_Transfer_new_jump
            (l ? l : x->u.jump);
}

// rename labels only for non-zero label arguments.
T Ssa_Transfer_renameLabels_call(T t, Label_t l) {
    Error_impossible ();
    return 0;
}

// how many successors a transfer have
static int Ssa_Transfer_numSuccs(T t) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF: {
            if (Label_equals(t->u.iff.truee, t->u.iff.falsee)) {
                Error_impossible ();
                return 0;
            }
            return 2;
        }
        case SSA_TRANS_JUMP:
            return 1;
        case SSA_TRANS_RETURN:
            return 0;
        case SSA_TRANS_THROW:
            return 0;
        case SSA_TRANS_CALL: {
            if (t->u.call.leave)
                return 2;
            return 2;
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

Set_t Ssa_Transfer_getDefIds(T t) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF: {
            return Set_new((Poly_tyEquals) Id_equals);
        }
        case SSA_TRANS_JUMP: {
            return Set_new((Poly_tyEquals) Id_equals);
        }
        case SSA_TRANS_RETURN: {
            return Set_new((Poly_tyEquals) Id_equals);
        }
        case SSA_TRANS_THROW: {
            return Set_new((Poly_tyEquals) Id_equals);
        }
        case SSA_STM_CALL: {
            if (t->u.call.dest)
                return Set_singleton((Poly_tyEquals) Id_equals, t->u.call.dest);
            return Set_new((Poly_tyEquals) Id_equals);
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

// whether or not the current transfer can jump to "l"
int Ssa_Transfer_canJumpTo(T t, Label_t l) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF: {
            return Label_equals(t->u.iff.truee, l)
                   || Label_equals(t->u.iff.falsee, l);
        }
        case SSA_TRANS_JUMP:
            return Label_equals(t->u.jump, l);
        case SSA_TRANS_RETURN:
            return 0;
        case SSA_TRANS_THROW:
            return 0;
        case SSA_TRANS_CALL: {
            return Label_equals(t->u.call.leave, l)
                   || Label_equals(t->u.call.normal, l);
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

// rename the id
T Ssa_Transfer_renameUse(T t, Id_t (*use)(Id_t)) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF:
            return Ssa_Transfer_new_if
                    (Ssa_Operand_renameUse(t->u.iff.cond, use), t->u.iff.truee, t->u.iff.falsee);
        case SSA_TRANS_JUMP:
            return t;
        case SSA_TRANS_RETURN:
            return Ssa_Transfer_new_return
                    (Ssa_Operand_renameUse(t->u.ret, use));
        case SSA_TRANS_THROW:
            return t;
        case SSA_TRANS_CALL: {
            List_t newArgs = List_new();
            List_t args = List_getFirst(t->u.call.args);

            while (args) {
                Ssa_Operand_t o = (Ssa_Operand_t) args->data;
                o = Ssa_Operand_renameUse(o, use);
                List_insertLast(newArgs, o);
                args = args->next;
            }
            return Ssa_Transfer_new_call(t->u.call.dest, t->u.call.name, newArgs, t->u.call.leave, t->u.call.normal);
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

static T Ssa_Transfer_renameUseDefNoPhiUse(T t) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF:
            return Ssa_Transfer_new_if
                    (Ssa_Operand_renameUse(t->u.iff.cond, gud.use), t->u.iff.truee, t->u.iff.falsee);
        case SSA_TRANS_JUMP:
            return t;
        case SSA_TRANS_RETURN: {
            T new;
            printf("return starting\n");

            Ssa_Operand_print(stdout, t->u.ret);
            new = Ssa_Transfer_new_return
                    (Ssa_Operand_renameUse(t->u.ret, gud.use));
            printf("return finished\n");
            return new;
        }
        case SSA_TRANS_THROW:
            return t;
        case SSA_TRANS_CALL: {
            List_t newArgs = List_new();
            List_t args = List_getFirst(t->u.call.args);

            while (args) {
                Ssa_Operand_t o = (Ssa_Operand_t) args->data;
                o = Ssa_Operand_renameUse(o, gud.use);
                List_insertLast(newArgs, o);
                args = args->next;
            }
            return Ssa_Transfer_new_call((t->u.call.dest) ?
                                         gud.def(t->u.call.dest) : 0, t->u.call.name, newArgs, t->u.call.leave,
                                         t->u.call.normal);
        }
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

void Ssa_Transfer_foreachUse(T t, void (*f)(Id_t)) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF:
            Ssa_Operand_foreachUse(t->u.iff.cond, f);
            return;
        case SSA_TRANS_JUMP:
            return;
        case SSA_TRANS_RETURN:
            Ssa_Operand_foreachUse(t->u.ret, f);
            return;
        case SSA_TRANS_THROW:
            return;
        case SSA_TRANS_CALL: {
            List_t l = List_getFirst(t->u.call.args);

            while (l) {
                Ssa_Operand_foreachUse(l->data, f);
                l = l->next;
            }
            return;
        }
        default:
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

void Ssa_Transfer_foreachDef(T t, void (*f)(Id_t)) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF:
            return;
        case SSA_TRANS_JUMP:
            return;
        case SSA_TRANS_RETURN:
            return;
        case SSA_TRANS_THROW:
            return;
        case SSA_TRANS_CALL: {
            if (t->u.call.dest) {
                f(t->u.call.dest);
            }
            return;
        }
        default:
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

T Ssa_Transfer_renameUse2Op(T t, O (*f)(Id_t)) {
    Assert_ASSERT(t);
    switch (t->kind) {
        case SSA_TRANS_IF:
            return Ssa_Transfer_new_if
                    (Ssa_Operand_renameUse2Op(t->u.iff.cond, f), t->u.iff.truee, t->u.iff.falsee);
        case SSA_TRANS_JUMP:
            return t;
        case SSA_TRANS_RETURN:
            return Ssa_Transfer_new_return
                    (Ssa_Operand_renameUse2Op(t->u.ret, f));
        case SSA_TRANS_THROW:
            return t;
        case SSA_TRANS_CALL:
            globalf = f;
            return Ssa_Transfer_new_call
                    (t->u.call.dest, t->u.call.name, List_map(t->u.call.args, (Poly_tyId) Ssa_Operand_renameUse2OpList),
                     t->u.call.leave, t->u.call.normal);
        default:
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

File_t Ssa_Transfer_print(File_t file, T t) {
    Assert_ASSERT(t);
    spacetab(file);
    switch (t->kind) {
        case SSA_TRANS_IF:
            fprintf(file, "%s", "if (");
            Ssa_Operand_print(file, t->u.iff.cond);
            fprintf(file, "%s", ", T=");
            fprintf(file, "%s", Label_toString(t->u.iff.truee));
            fprintf(file, "%s", ", F=");
            fprintf(file, "%s", Label_toString(t->u.iff.falsee));
            fprintf(file, "%s", ")");
            break;
        case SSA_TRANS_JUMP:
            fprintf(file, "%s", "jmp ");
            fprintf(file, "%s", Label_toString(t->u.jump));
            break;
        case SSA_TRANS_RETURN:
            fprintf(file, "%s", "return ");
            Ssa_Operand_print(file, t->u.ret);
            break;
        case SSA_TRANS_THROW:
            fprintf(file, "%s", "throw");
            break;
        case SSA_TRANS_CALL:
            if (t->u.call.dest) {
                fprintf(file, "%s", Id_toString(t->u.call.dest));
                fprintf(file, " = ");
            }
            fprintf(file, "%s", Id_toString(t->u.call.name));
            fprintf(file, "(");
            List_print(t->u.call.args, ", ", file, (Poly_tyListPrint) Ssa_Operand_print);
            fprintf(file, "%s", ")");
            fprintf(file, "LEAVE=%s, NORMAL=%s", t->u.call.leave ?
                                                 (Label_toString(t->u.call.leave)) : ("<NONE>"),
                    Label_toString(t->u.call.normal));
            break;
        default:
            Error_impossible ();
            break;
    }
    if (semi)
        fprintf(file, "\n");
    else
        fprintf(file, "\\n");
    return file;
}


///////////////////////////////////////////////////////
/* basic block */
B Ssa_Block_new(Label_t label, List_t stms, T t) {
    B b;

    Mem_NEW(b);
    b->label = label;
    b->stms = stms;
    b->transfer = t;
    return b;
}

int Ssa_Block_equals(B b1, B b2) {
    Assert_ASSERT(b1);
    Assert_ASSERT(b2);

    return Label_equals(b1->label, b2->label);
}

int Ssa_Block_canJumpTo(B b, Label_t l) {
    Assert_ASSERT(b);
    Assert_ASSERT(l);

    return Ssa_Transfer_canJumpTo(b->transfer, l);
}

File_t Ssa_Block_print(File_t file, B b) {
    Assert_ASSERT(b);

    fprintf(file, "%s", Label_toString(b->label));
    fprintf(file, "%s", ":\n");
    semi = 1;
    List_foldl(b->stms, file, (Poly_tyFold) Ssa_Stm_print);
    Ssa_Transfer_print(file, b->transfer);
    fprintf(file, "%s", "\n");
    return file;
}

B Ssa_Block_renameUseDefNoPhiUse(B b, Id_t (*use)(Id_t), Id_t (*def)(Id_t)) {
    List_t newStms;
    T newTransfer;

    gud.use = use;
    gud.def = def;

    printf("to here?\n");
    newStms = List_map(b->stms, (Poly_tyId) Ssa_Stm_renameUseDefNoPhiUse);
    //newStms = b->stms;
    /* printf ("debug:\n"); */
    /* file = stdout; */
    /* List_foreach (newStms, Ssa_Stm_print); */
    // there are only uses in any transfers.
    printf("2?\n");

    newTransfer = Ssa_Transfer_renameUseDefNoPhiUse
            (b->transfer);
    printf("block returns\n");
    gud.use = 0;
    gud.def = 0;
    return Ssa_Block_new(b->label, newStms, newTransfer);
}

void Ssa_Block_renamePhiArgPre(B current, B predessor, void (*f)(B, B, Id_t)) {
    List_t stms = List_getFirst(current->stms);

    //printf ("handling block: %s\n"
    //      , Label_toString(current->label));
    while (stms) {
        S s = (S) stms->data;
        //Ssa_Stm_print (s);
        if (s->kind != SSA_STM_PHI) {
            //printf ("what the first one: %d\n", s->kind);
            break;
        }

        {
            Id_t olddest = s->u.phi.dest;
            //printf ("%s called\n", __FILE__);
            f(current, predessor, olddest);
        }

        stms = stms->next;
    }
    return;
}

void Ssa_Block_foreachDef(B b, void (*f)(Id_t)) {
    List_t stms = List_getFirst(b->stms);

    while (stms) {
        S s = (S) stms->data;

        Ssa_Stm_foreachDef(s, f);
        stms = stms->next;
    }
    // transfer may also contain definitions
    Ssa_Transfer_foreachDef(b->transfer, f);
    return;
}

void Ssa_Block_foreachUse(B b, void (*f)(Id_t)) {
    List_t stms = List_getFirst(b->stms);

    while (stms) {
        S s = (S) stms->data;

        Ssa_Stm_foreachUse(s, f);
        stms = stms->next;
    }
    Ssa_Transfer_foreachUse(b->transfer, f);
    return;
}


B Ssa_Block_renamePhiArgs(B b, List_t subst) {
    List_t newStms;

    //
    globalSubstPhi = subst;
    newStms = List_map(b->stms, (Poly_tyId) Ssa_Stm_renamePhiArgs);
    return Ssa_Block_new(b->label, newStms, b->transfer);
}


Plist_t Ssa_Block_plist(B b) {
    Assert_ASSERT(b);
    return (Label_plist(b->label));
}

static Set_t Ssa_Block_getDefIdsTraced(B b) {
    //Set_t set = Set_new ((Poly_tyEquals)Id_equals);
    HashSet_t set = HashSet_new((long (*)(Poly_t)) Id_hashCode, (Poly_tyEquals) Id_equals);
    List_t stms;

    Assert_ASSERT(b);

    stms = List_getFirst(b->stms);
    while (stms) {
        S s = (S) stms->data;

        // Several strategies for set union operations:
        //   1. set = Set_union (...)
        //   2. Set_unionVoid (...)
        //   3. HashSet_unionVoidSet (...)
        // Several interesting observations:
        //   * For a test of 5000 lines of code, (2) is
        //     about 30X faster than (1).
        //   * For a test of 100000 lines of code, (2)
        //     would require
        //     about 205 seconds to run (on my XP machine
        //     with 4G RAM).
        //   * For a test of 100000 lines of code, (3) would
        //     require about 0.75 seconds to run (same machine)
        HashSet_unionVoidSet(set, Ssa_Stm_getDefIds(s));
        stms = stms->next;
    }
    // "transfer" can define variables
    HashSet_unionVoidSet
            (set, Ssa_Transfer_getDefIds(b->transfer));
    return HashSet_toSet((Poly_tyEquals) Id_equals, set);
}

static void dumb(Poly_t p) {}

Set_t Ssa_Block_getDefIds(B b) {
    Set_t r;

    Trace_TRACE("getDefIds", Ssa_Block_getDefIdsTraced, (b), dumb, r, dumb);
    return r;
}

File_t Ssa_Block_printForDot(File_t file, B b) {
    List_t stms;

    Assert_ASSERT(b);

    stms = List_getFirst(b->stms);
    fprintf(file, "%s", Label_toString(b->label));
    fprintf(file, ":\\n");
    semi = 0;
    while (stms) {
        S s = (S) stms->data;

        Ssa_Stm_print(file, s);
        stms = stms->next;
    }
    Ssa_Transfer_print(file, b->transfer);
    return file;
}

int Ssa_Block_numSuccs(B b) {
    Assert_ASSERT(b);

    return Ssa_Transfer_numSuccs(b->transfer);
}


//////////////////////////////////////////////////////
// functions
F Ssa_Fun_new(Atype_t type, Id_t name, List_t args,
              List_t decs, List_t blocks,
              Id_t retId,
              Label_t entry, Label_t exitt) {
    F f;

    Mem_NEW (f);
    f->type = type;
    f->name = name;
    f->args = args;
    f->decs = decs;
    f->blocks = blocks;
    f->retId = retId;
    f->entry = entry;
    f->exitt = exitt;
    return f;
}

File_t Ssa_Fun_print(File_t file, F f) {
    Assert_ASSERT(f);
    fprintf(file, "%s", Atype_toString(f->type));
    fprintf(file, " ");
    fprintf(file, "%s", Id_toString(f->name));
    fprintf(file, "(");
    List_foldl(f->args, file, (Poly_tyFold) Dec_printAsArg);
    fprintf(file, ")\n{\n");
    List_foldl(f->decs, file, (Poly_tyFold) Dec_printAsLocal);
    fprintf(file, "\n");
    fprintf(file, "Fentry = ");
    fprintf(file, "%s", Label_toString(f->entry));
    fprintf(file, ", Fexit = ");
    fprintf(file, "%s", Label_toString(f->exitt));
    fprintf(file, "\n");
    List_foldl(f->blocks, file, (Poly_tyFold) Ssa_Block_print);
    fprintf(file, "}\n\n");
    return file;
}

void Ssa_Fun_toDot(F f, String_t name) {
    Graph_t g = Ssa_Fun_toGraph(f);

    if (!dotname)
        Error_impossible ();

    Graph_toJpgWithName(g, (Poly_tyPrint) Ssa_Block_printForDot, name);
    return;
}

Graph_t Ssa_Fun_toGraph(F f) {
    Graph_t g;
    List_t blocks;

    Assert_ASSERT(f);

    g = Graph_newWithName((Poly_tyEquals) Ssa_Block_equals, Id_toString(f->name));

    // insert all vertex
    blocks = List_getFirst(f->blocks);
    while (blocks) {
        B b = (B) blocks->data;

        Graph_insertVertex(g, b);
        blocks = blocks->next;
    }

    // insert all edges
    blocks = List_getFirst(f->blocks);
    while (blocks) {
        B b = (B) blocks->data;
        T trans = b->transfer;

        switch (trans->kind) {
            case SSA_TRANS_IF: {
                B tb, fb;

                tb = Ssa_Fun_searchLabel(f, trans->u.iff.truee);
                fb = Ssa_Fun_searchLabel(f, trans->u.iff.falsee);
                if (tb)
                    Graph_insertEdge(g, b, tb);
                if (fb)
                    Graph_insertEdge(g, b, fb);
                break;
            }
            case SSA_TRANS_JUMP: {
                B jmp;

                jmp = Ssa_Fun_searchLabel(f, trans->u.jump);
                if (jmp)
                    Graph_insertEdge(g, b, jmp);
                break;
            }
            case SSA_TRANS_RETURN:
                break;
            case SSA_TRANS_THROW:
                break;
            case SSA_TRANS_CALL: {
                B leaveB = 0, normalB = 0;

                leaveB = Ssa_Fun_searchLabel
                        (f, trans->u.call.leave);
                normalB = Ssa_Fun_searchLabel
                        (f, trans->u.call.normal);
                if (leaveB)
                    Graph_insertEdge(g, b, leaveB);
                if (normalB)
                    Graph_insertEdge(g, b, normalB);
                break;
            }
            default:
                Error_impossible ();
                break;
        }
        blocks = blocks->next;
    }
    return g;
}

// number of predessors for b in f
int Ssa_Fun_numPreds(F f, B b) {
    List_t blocks;
    int num = 0;

    Assert_ASSERT(f);
    Assert_ASSERT(b);
    blocks = List_getFirst(f->blocks);
    while (blocks) {
        B b = (B) blocks->data;
        T transfer = b->transfer;
        //switch (transfer->kind){
        fprintf(stderr, "%s", "error");
        exit(0);
        //}
        blocks = blocks->next;
    }
    return 0;
}

// Return 0 when search fails (when the label is in
// some dead block).
B Ssa_Fun_searchLabel(F f, Label_t l) {
    List_t blocks;

    Assert_ASSERT(f);
    Assert_ASSERT(l);
    blocks = List_getFirst(f->blocks);
    while (blocks) {
        B b = (B) blocks->data;

        if (Label_equals(b->label, l))
            return b;
        blocks = blocks->next;
    }
    return 0;
}




//////////////////////////////////////////////////////
// programs
P Ssa_Prog_new(List_t classes, List_t funcs) {
    P p;

    Mem_NEW (p);
    p->classes = classes;
    p->funcs = funcs;
    return p;
}

File_t Ssa_Prog_print(File_t file, P x) {
    Assert_ASSERT(file);
    Assert_ASSERT(x);

    List_foldl(x->classes, file, (Poly_tyFold) Class_print);
    fprintf(file, "\n");
    List_foldl(x->funcs, file, (Poly_tyFold) Ssa_Fun_print);
    return file;
}

static String_t gfname = 0;

static void progToDotEach(F f) {
    Ssa_Fun_toDot(f, gfname);
    return;
}

void Ssa_Prog_toDot(P x, String_t fname) {
    Assert_ASSERT(x);
    Assert_ASSERT(fname);
    gfname = fname;
    List_foreach(x->funcs, (Poly_tyVoid) progToDotEach);
    gfname = 0;
    return;
}


#undef B
#undef F
#undef M
#undef O
#undef P
#undef S
#undef T

