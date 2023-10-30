#include "../lib/mem.h"
#include "../lib/hash.h"
#include "../lib/assert.h"
#include "../lib/int.h"
#include "../lib/random.h"
#include "../lib/property-list.h"
#include "label.h"

#define T Label_t

static int counter = 0;

struct T {
    int count;
    int hashCode;
    Plist_t plist;
};

T Label_new() {
    T x;

    Mem_NEW (x);
    x->count = counter++;
    x->hashCode = Random_nextInt();
    x->plist = Plist_new();
    return x;
}

int Label_hashCode(T x) {
    Assert_ASSERT(x);
    return x->hashCode;
}

String_t Label_toString(T x) {
    Assert_ASSERT (x);
    return String_concat("L_", Int_toString(x->count), 0);
}

int Label_equals(T x, T y) {
    Assert_ASSERT(x);
    Assert_ASSERT(y);
    return x == y;
}

Plist_t Label_plist(T x) {
    Assert_ASSERT(x);
    return x->plist;
}

void Label_print(T x) {
    Assert_ASSERT(x);
    printf("%s_%d", "L", x->count);
}

#undef T
