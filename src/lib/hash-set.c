#include "assert.h"
#include "mem.h"
#include "list.h"
#include "hash.h"
#include "hash-set.h"

#define T HashSet_t

struct T {
    Hash_t hash;
};

T HashSet_new(long (*hashCode)(Poly_t), Poly_tyEquals equals) {
    T set;

    Mem_NEW(set);
    // we don't use "dup"
    set->hash = Hash_new(hashCode, equals, 0);
    return set;
}

long HashSet_exists(T set, Poly_t x) {
    Assert_ASSERT(set);

    return (long) Hash_lookup(set->hash, x);
}

void HashSet_delete(T set, Poly_t x) {
    Error_impossible ();
    return;
}

// delete all items x for pred(x) is true
void HashSet_deleteAll(T set, Poly_tyPred pred) {
    Error_impossible ();
    return;
}

void HashSet_foreach(T set, Poly_tyVoid f) {
    Assert_ASSERT(set);

    Error_impossible ();
    return;
}

long HashSet_isEmpty(T set) {
    Assert_ASSERT(set);

    Error_impossible ();
    return 1;
}

// Remove one element from a set.
Poly_t HashSet_removeOne(T set) {
    Assert_ASSERT(set);

    Error_impossible ();

    return 0;
}

void HashSet_insert(T set, Poly_t x) {
    if (Hash_lookup(set->hash, x))
        return;

    Hash_insert(set->hash, x, (Poly_t) 1);
    return;
}

int HashSet_size(T set) {
    Assert_ASSERT(set);

    Error_impossible ();
    return 0;
}

T HashSet_fromList(Poly_tyEquals equals, List_t l) {
    T set;

    Error_impossible ();
    return 0;
}

T HashSet_singleton(Poly_tyEquals equals, Poly_t x) {
    T set;

    Error_impossible ();
    return 0;
}

List_t HashSet_toList(T set) {
    Assert_ASSERT(set);
    Error_impossible ();
    return 0;
}

T HashSet_intersection(T set1, T set2) {
    T newSet;
    List_t p;

    Assert_ASSERT(set1);
    Assert_ASSERT(set2);

    Error_impossible ();
    return 0;
}

T HashSet_union(T set1, T set2) {
    T newSet;
    List_t p;

    Assert_ASSERT(set1);
    Assert_ASSERT(set2);

    Error_impossible ();
    return newSet;
}

//////////////////////////////////////////////////
// HashSet_unionVoid
static T gset = 0;

static void localForeach(Poly_t x) {
    HashSet_insert(gset, x);
}

void HashSet_unionVoid(T set1, T set2) {
    List_t p;

    Assert_ASSERT(set1);
    Assert_ASSERT(set2);

    gset = set1;
    Hash_foreach(set2->hash, localForeach);
    set1 = gset;
    gset = 0;
    return;
}

void HashSet_unionVoidSet(T set1, Set_t set2) {
    List_t p;

    Assert_ASSERT(set1);
    Assert_ASSERT(set2);

    p = List_getFirst(Set_toList(set2));
    while (p) {
        HashSet_insert(set1, p->data);
        p = p->next;
    }
    return;
}

int HashSet_equals(T set1, T set2) {
    T newSet;
    List_t p;

    Error_impossible ();
    return 1;
}

Set_t HashSet_toSet(Poly_tyEquals eqs, T set) {
    return Set_fromList(eqs, Hash_keyToList(set->hash));
}

#undef T

