#include "../lib/assert.h"
#include "../lib/mem.h"
#include "../lib/hash.h"
#include "../lib/tuple.h"
#include "../lib/stack.h"
#include "../control/region.h"
#include "../control/error-msg.h"
#include "env.h"

#define Cenv_t Hash_t
#define Venv_t Stack_t
#define Senv_t Hash_t

Env_Binding_t Env_Binding_new(Type_t ty, AstId_t fresh) {
    Env_Binding_t t;

    Mem_NEW (t);
    t->type = ty;
    t->fresh = fresh;
    return t;
}

/////////////////////////////////////////////////////
// Env binding. For "Cenv".
typedef struct {
    AstId_t fresh;
    // List<AstId_t>, old ids, must keep order
    List_t fields;
} *CEnv_Binding_t;

static CEnv_Binding_t
CEnv_Binding_new(AstId_t fresh, List_t fs) {
    CEnv_Binding_t t;

    Mem_NEW (t);
    t->fresh = fresh;
    t->fields = fs;
    return t;
}

/////////////////////////////////////////////////////
// error-reporting util
static void error(String_t s, Region_t r) {
    ErrorMsg_elabError(s, r);
}

static void error_dupClassName(Id_t old, AstId_t current) {
    error(String_concat
                  ("duplicate class name: ",
                   AstId_toString(current),
                   0),
          AstId_getRegion(current));
    return;
}

/////////////////////////////////////////////////
// class name env
static Cenv_t cenv = 0;

void Cenv_init() {
    cenv = Hash_new((tyHashCode) AstId_hashCode,
                    (Poly_tyEquals) AstId_equals,
                    (tyDup) error_dupClassName);
    return;
}

AstId_t Cenv_insert(AstId_t id, List_t fs) {
    AstId_t fresh = AstId_newNoName();

    Hash_insert(cenv,
                id,
                CEnv_Binding_new(fresh, fs));

    return fresh;
}

AstId_t Cenv_lookup(AstId_t id) {
    CEnv_Binding_t b = (CEnv_Binding_t) Hash_lookup(cenv, id);
    return b->fresh;
}

AstId_t Cenv_lookupMustExist(AstId_t id) {
    CEnv_Binding_t b = (CEnv_Binding_t) Hash_lookup(cenv, id);
    if (!b) {
        error(String_concat
                      ("undefined class name: ",
                       AstId_toString(id),
                       0),
              AstId_getRegion(id));
        return id;
    }
    return b->fresh;
}

List_t Cenv_lookupFields(AstId_t id) {
    CEnv_Binding_t b = (CEnv_Binding_t) Hash_lookup(cenv, id);
    if (!b) {
        error(String_concat
                      ("undefined class name: ",
                       AstId_toString(id),
                       0),
              AstId_getRegion(id));
        return List_new();
    }
    return b->fields;
}

//////////////////////////////////////////////////////
// Variable env.
//////////////////////////////////////////////////////
static Venv_t venv = 0;

static void error_dupVar(AstId_t old, AstId_t cur) {
    error(String_concat
                  ("duplicate variable: ",
                   AstId_toString(cur),
                   0),
          AstId_dest(cur));
    return;
}

void Venv_init() {
    Hash_t h;

    venv = Stack_new();
    h = Hash_new((tyHashCode) AstId_hashCode,
                 (Poly_tyEquals) AstId_equals,
                 (tyDup) error_dupVar);
    /* build all libraries */
    {
        AstId_t idIo_printi = AstId_fromString("printi",
                                               Region_bogus());
        Hash_insert(h,
                    idIo_printi,
                    Env_Binding_new
                            (Type_new_fun(Type_new_product
                                                  (Type_new_int(),
                                                   0),
                                          Type_new_int()),
                             idIo_printi));
        Stack_push(venv, h);
    }
    {
        AstId_t idIo_print = AstId_fromString("prints",
                                              Region_bogus());
        Hash_insert(h,
                    idIo_print,
                    Env_Binding_new
                            (Type_new_fun(Type_new_product
                                                  (Type_new_string(),
                                                   0),
                                          Type_new_int()),
                             idIo_print));
        Stack_push(venv, h);
    }
    return;
}

AstId_t Venv_insert(AstId_t id, Type_t ty) {
    Hash_t h;
    AstId_t fresh = AstId_newNoName();

    Assert_ASSERT(id);
    Assert_ASSERT(ty);
    h = Stack_getTop(venv);
    Hash_insert(h,
                id,
                Env_Binding_new(ty,
                                fresh));
    return fresh;
}

Env_Binding_t Venv_lookup(AstId_t id) {
    List_t list = List_getFirst(venv);
    Hash_t hash;
    Env_Binding_t r;

    while (list) {
        hash = (Hash_t) list->data;
        r = Hash_lookup(hash, id);
        if (r)
            return r;
        list = list->next;
    }
    return 0;
}

Env_Binding_t Venv_lookupMustExist(AstId_t id) {
    Env_Binding_t b;

    b = Venv_lookup(id);
    if (!b) {
        error(String_concat
                      ("unbound variable: ",
                       AstId_toString(id),
                       0),
              AstId_dest(id));
        return Env_Binding_new(Type_new_int(),
                               AstId_bogus());
    }
    return b;
}

void Venv_enterScope() {
    Hash_t h;

    h = Hash_new((tyHashCode) AstId_hashCode,
                 (Poly_tyEquals) AstId_equals,
                 (tyDup) error_dupVar);
    Stack_push(venv, h);
}

void Venv_exitScope() {
    Stack_pop(venv);
}

//////////////////////////////////////////////////
// class fields env.
static Senv_t senv = 0;

static int Skey_hashCode(Tuple_t k) {
    Assert_ASSERT(k);
    return AstId_hashCode(Tuple_first(k))
           + AstId_hashCode(Tuple_second(k));
}

static int Skey_equals(Tuple_t k1, Tuple_t k2) {
    Assert_ASSERT (k1);
    Assert_ASSERT (k2);
    return AstId_equals(Tuple_first(k1), Tuple_first(k2))
           && AstId_equals(Tuple_second(k1), Tuple_second(k2));
}

static void error_dupField(Tuple_t k, AstId_t id) {
    AstId_t sid, fid;

    Assert_ASSERT(k);
    sid = Tuple_first(k);
    fid = Tuple_second(k);

    error(String_concat
                  ("duplicate class field: ",
                   AstId_toString(fid),
                   " in ",
                   AstId_toString(sid),
                   0),
          AstId_dest(fid));
}

void Senv_init() {
    senv = Hash_new((tyHashCode) Skey_hashCode, (Poly_tyEquals) Skey_equals, (tyDup) error_dupField);
}

AstId_t Senv_insert(AstId_t sid, AstId_t fid, Type_t ty) {
    AstId_t fresh = AstId_newNoName();

    Hash_insert(senv, Tuple_new(sid, fid), Env_Binding_new(ty, fresh));
    return fresh;
}

Env_Binding_t Senv_lookupMustExist(AstId_t sid, AstId_t fid) {
    Env_Binding_t bd =
            Hash_lookup(senv, Tuple_new(sid, fid));
    if (!bd) {
        error(String_concat
                      ("unbound class field: ",
                       AstId_toString(fid),
                       0),
              AstId_dest(fid));
        return Env_Binding_new(Type_new_int(),
                               fid);
    }
    return bd;
}

#undef Cenv_t
#undef Venv_t
#undef Senv_t
