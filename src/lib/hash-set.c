#include "hash-set.h"
#include "hash.h"
#include "list.h"
#include "mem.h"
#include "unused.h"
#include <assert.h>

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
    assert(set);
    Poly_t v = Hash_lookup(set->hash, x);
    return (long) v;
}

void HashSet_delete(T set, Poly_t x) {
    UNUSED(set);
    UNUSED(x);
    Error_impossible();
    //    return;
}
//
//// delete all items x for pred(x) is true
//void HashSet_deleteAll(T set, Poly_tyPred pred) {
//    UNUSED(set);
//    UNUSED(pred);
//    Error_impossible();
//    //    return;
//}

void HashSet_foreach(T set, Poly_tyVoid f) {
    assert(set);
    UNUSED(set);
    UNUSED(f);

    Error_impossible();
    //    return;
}

long HashSet_isEmpty(T set) {
    assert(set);

    Error_impossible();
    return 1;
}

// Remove one element from a set.
Poly_t HashSet_removeOne(T set) {
    assert(set);

    Error_impossible();

    return 0;
}

void HashSet_insert(T set, Poly_t x) {
    if (Hash_lookup(set->hash, x))
        return;

    Hash_insert(set->hash, x, (Poly_t) 1);
    //    return;
}

//int HashSet_size(T set) {
//    assert(set);
//
//    Error_impossible();
//    return 0;
//}

T HashSet_fromList(Poly_tyEquals equals, List_t l) {
    UNUSED(equals);
    UNUSED(l);

    Error_impossible();
    return 0;
}

T HashSet_singleton(Poly_tyEquals equals, Poly_t x) {
    UNUSED(equals);
    UNUSED(x);

    Error_impossible();
    return 0;
}

List_t HashSet_toList(T set) {
    assert(set);
    Error_impossible();
    return 0;
}

T HashSet_intersection(T set1, T set2) {
    assert(set1);
    assert(set2);

    Error_impossible();
    return 0;
}

T HashSet_union(T set1, T set2) {
    T newSet = 0;
    assert(set1);
    assert(set2);

    Error_impossible();
    return newSet;
}

//////////////////////////////////////////////////
// HashSet_unionVoid
static T gset = 0;

static void localForeach(Poly_t x) {
    HashSet_insert(gset, x);
}

void HashSet_unionVoid(T set1, T set2) {
    assert(set1);
    assert(set2);

    gset = set1;
    Hash_foreach(set2->hash, localForeach);
    set1 = gset;
    gset = 0;
    //    return;
}

void HashSet_unionVoidSet(T set1, Set_t set2) {
    List_t p;

    assert(set1);
    assert(set2);

    p = List_getFirst(Set_toList(set2));
    while (p) {
        HashSet_insert(set1, p->data);
        p = p->next;
    }
    //    return;
}

//static int HashSet_equals(T set1, T set2) {
//    UNUSED(set1);
//    UNUSED(set2);
//
//    Error_impossible();
//    return 1;
//}

Set_t HashSet_toSet(Poly_tyEquals eqs, T set) {
    return Set_fromList(eqs, Hash_keyToList(set->hash));
}

#undef T
