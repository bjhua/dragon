#include <assert.h>
#include "../lib/mem.h"
#include "ast-id.h"

#define T AstId_t

struct T {
    Id_t id;
    Region_t region;
};

static T AstId_new(Id_t id, Region_t r) {
    T t;
    Mem_NEW (t);
    t->id = id;
    t->region = r;
    return t;
}

T AstId_fromString(String_t s, Region_t r) {
    return AstId_new(Id_fromString(s), r);
}

T AstId_bogus() {
    return AstId_new(Id_bogus(), Region_bogus());
}

T AstId_newNoName() {
    return AstId_new(Id_newNoName(), Region_bogus());
}

Id_t AstId_toId(T id) {
    assert(id);
    return id->id;
}

long AstId_equals(T id1, T id2) {
    assert(id1);
    assert(id2);
    return Id_equals(id1->id, id2->id);
}

long AstId_hashCode(T t) {
    assert (t);
    return Id_hashCode(t->id);
}

String_t AstId_toString(T t) {
    assert(t);
    return Id_toString(t->id);
}

Region_t AstId_dest(T t) {
    assert(t);
    return t->region;
}

Region_t AstId_getRegion(T t) {
    assert(t);
    return t->region;
}

Plist_t AstId_plist(T t) {
    assert(t);
    return Id_plist(t->id);
}

void AstId_print(T t) {
    assert(t);
    Id_print(t->id);
}


#undef T
