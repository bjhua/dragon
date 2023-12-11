#include "hash.h"
#include "double.h"
#include "error.h"
#include "int.h"
#include "list.h"
#include "mem.h"
#include "string.h"
#include "todo.h"
#include "triple.h"
#include "unused.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define INIT_SIZE 4096
#define INIT_MASK 4095
#define INIT_LOAD_FACTOR (0.25)

#define T Hash_t
#define K Poly_t
#define V Poly_t

struct T {
    List_t *buckets;

    long (*hashCode)(K);

    long (*equals)(K, K);

    void (*dup)(K, K);

    //void (*notFound)(K);
    long numItems;
    long size;
    long mask;
    double load;
};

//T Hash_newRaw(long (*hashCode)(K), long (*equals)(K, K)) {
//    return Hash_new(hashCode, equals, 0);
//}

T Hash_new(long (*hashCode)(K), long (*equals)(K, K), void (*dup)(K, K)) {
    T h;

    Mem_NEW(h);
    Mem_NEW_SIZE(h->buckets, INIT_SIZE);

    for (long i = 0; i < INIT_SIZE; i++)
        *(h->buckets + i) = List_new();
    h->hashCode = hashCode;
    h->equals = equals;
    h->dup = dup;
    //h->notFound = notFound;
    h->numItems = 0;
    h->size = INIT_SIZE;
    h->mask = INIT_MASK;
    h->load = INIT_LOAD_FACTOR;
    return h;
}

static double currentLoad(T h) {
    return 1.0 * (double) h->numItems / (double) h->size;
}

//T Hash_clone(T old) {
//    T h;
//    int i;
//    int newSize = old->size * 2;
//
//    Mem_NEW(h);
//    Mem_NEW_SIZE(h->buckets, newSize);
//
//    for (i = 0; i < newSize; i++)
//        *(h->buckets + i) = List_new();
//    h->numItems = old->numItems + 1;
//    h->size = newSize;
//    h->mask = newSize - 1;
//    h->hashCode = old->hashCode;
//    h->equals = old->equals;
//    return h;
//}

typedef struct {
    long insertions;
    long lookups;
    long links;
    long longest;
    long maxSize;
    double maxLoad;
} Status_t;

static Status_t all = {0, 0, 0, 0, 0, 0.0};

void Hash_insert(T h, K k, V v) {
    long initHc, hc;
    List_t list;
    K result;

    assert(h);
    all.insertions++;
    if (Hash_lookupCand(h, k, &result)) {
        if (h->dup) {
            h->dup(result, k);
        }
    }
    initHc = h->hashCode(k);
    hc = initHc & (h->mask);
    list = *(h->buckets + hc);
    List_insertFirst(list,
                     Triple_new(k,
                                (Poly_t) initHc, v));
    h->numItems++;
    if (currentLoad(h) >= h->load) {
        List_t *oldBuckets, *newBuckets;
        long oldSize, newSize;

        oldBuckets = h->buckets;
        oldSize = h->size;
        newSize = oldSize * 2;
        Mem_NEW_SIZE(newBuckets, newSize);
        for (long i = 0; i < newSize; i++)
            *(newBuckets + i) = List_new();
        h->buckets = newBuckets;
        h->size = newSize;
        h->mask = newSize - 1;
        for (long i = 0; i < oldSize; i++) {
            List_t listx = *(oldBuckets + i);
            listx = List_getFirst(listx);
            while (listx) {
                Triple_t t = (Triple_t) (listx->data);
                Poly_t p = Triple_second(t);
                long initHcx = (long) p;
                List_insertLast(*(newBuckets + (initHcx & h->mask)), t);
                list = list->next;
            }
        }
    }
    return;
}

static int longestChain(T h) {
    int max = 0;
    int current;
    int i;

    for (i = 0; i < h->size; i++) {
        current = List_size(*(h->buckets + i));
        if (current > max)
            max = current;
    }
    return max;
}

static int numEmptyBuckets(T h) {
    int empty = 0;
    int i;
    assert(h);

    for (i = 0; i < h->size; i++) {
        if (List_isEmpty(*(h->buckets + i)))
            empty++;
    }
    return empty;
}

String_t Hash_status(T h) {
    int empty;
    String_t s;

    assert(h);
    empty = numEmptyBuckets(h);
    s = String_concat("number of items are: ",
                      Int_toString(h->numItems),
                      "\nnumber of buckets: ",
                      Int_toString(h->size),
                      "\nnumber of empty buckets: ",
                      numEmptyBuckets(h),
                      "\nnumber of not empty buckets: ",
                      Int_toString(h->size - empty),
                      "\nlongest chain size: ",
                      Int_toString(longestChain(h)),
                      "\ncrrent load factor: ",
                      Double_toString(h->load),
                      "\ndefault load factor: ",
                      Double_toString(INIT_LOAD_FACTOR),
                      0);
    return s;
}

V Hash_lookup(T h, K x) {
    return Hash_lookupCand(h, x, 0);
}

V Hash_lookupCand(T h, K x, K *result) {
    long hc;
    long link = 0;
    double load;
    List_t list;

    assert(h);

    all.lookups++;
    long size = Hash_size(h);
    if (size > all.maxSize)
        all.maxSize = size;
    long ns = Hash_numItems(h);
    load = 1.0 * (double) ns / (double) size;
    if (load > all.maxLoad)
        all.maxLoad = load;

    hc = h->hashCode(x);
    hc &= h->mask;

    assert((hc >= 0 && hc < h->size));

    list = List_getFirst(*(h->buckets + hc));
    while (list) {
        Triple_t t = (Triple_t) (list->data);
        K key = Triple_first(t);
        V value = Triple_third(t);

        link++;
        if (h->equals(x, key)) {
            all.links += link;
            if (link > all.longest)
                all.longest = link;
            if (result)
                *result = key;
            return value;
        }
        list = list->next;
    }
    all.links += link;
    if (link > all.longest)
        all.longest = link;
    return 0;
}

V Hash_lookupOrInsert(T h, K k, V (*gen)(K)) {
    V r;

    if ((r = Hash_lookup(h, k)))
        return r;
    r = gen(k);
    Hash_insert(h, k, r);
    return r;
}

void Hash_delete(T h, K k) {
    UNUSED(h);
    UNUSED(k);
    TODO;
}

long Hash_size(T h) {
    assert(h);
    return h->size;
}

long Hash_numItems(T h) {
    assert(h);
    return h->numItems;
}

void Hash_foreach(T h, void (*f)(K)) {
    int index;
    List_t list;

    for (index = 0; index < h->size; index++) {
        list = List_getFirst(*(h->buckets + index));
        while (list) {
            Triple_t t = (Triple_t) (list->data);
            K key = Triple_first(t);

            f(key);
            list = list->next;
        }
    }
    return;
}

List_t Hash_keyToList(T h) {
    int index;
    List_t list;
    List_t result = List_new();

    for (index = 0; index < h->size; index++) {
        list = List_getFirst(*(h->buckets + index));
        while (list) {
            Triple_t t = (Triple_t) (list->data);
            K key = Triple_first(t);

            List_insertLast(result, key);
            list = list->next;
        }
    }
    return result;
}

void Hash_statusAll() {
    printf("%s\n", "Hash table status:");
    printf("  Num of insertions: %ld\n", all.insertions);
    printf("  Num of lookups   : %ld\n", all.lookups);
    printf("  Num of links     : %ld\n", all.links);
    printf("  Longest chain    : %ld\n", all.longest);
    printf("  Max hash size    : %ld\n", all.maxSize);
    printf("  Max load factor  : %lf\n", all.maxLoad);
    printf("  Average position : %lf\n",
           1.0 * (double) all.links / (double) all.lookups);
    return;
}

#undef V
#undef K
#undef T
