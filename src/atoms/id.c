#include "id.h"
#include "../lib/hash.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include "../lib/property-list.h"
#include "../lib/random.h"
#include <assert.h>

#define T Id_t

/* table: String_t -> Id_t
 */
static Hash_t table = 0;

static int counter = 0;

struct T {
    String_t name;
    String_t newName;
    long hashCode;
    Plist_t plist;
};


static T Id_create(String_t s) {
    T x;

    assert(s);
    Mem_NEW(x);
    x->name = s;
    x->newName = 0;
    x->hashCode = String_hashCode(s);
    x->plist = Plist_new();
    return x;
}

T Id_bogus(void) {
    return Id_create("<bogus>");
}

T Id_fromString(String_t s) {
    T x;

    assert(s);
    x = Hash_lookupOrInsert(table, s, (tyKV) Id_create);
    return x;
}

T Id_newNoName(void) {
    T x;
    Mem_NEW(x);
    x->name = 0;
    x->newName = String_concat("x_",
                               Int_toString(counter++),
                               0);
    x->hashCode = Random_nextInt();
    x->plist = Plist_new();
    return x;
}

long Id_hashCode(T x) {
    assert(x);
    return x->hashCode;
}

String_t Id_toString(T x) {
    assert(x);
    assert(((x->name == 0) && (x->newName == 0)));
    return (x->name) ? (x->name) : (x->newName);
}

void Id_init(void) {
    table = Hash_new((tyHashCode) String_hashCode, (Poly_tyEquals) String_equals
                     // should never call this function.
                     ,
                     0);
}

long Id_equals(T x, T y) {
    assert(x);
    assert(y);
    return x == y;
}

Plist_t Id_plist(T x) {
    assert(x);
    return x->plist;
}

void Id_print(T x) {
    assert(x);
    assert((x->name == 0) && (x->newName == 0));
    printf("%s", (x->name) ? (x->name) : (x->newName));
}


#undef T
