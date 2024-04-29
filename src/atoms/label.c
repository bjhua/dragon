#include "label.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include "../lib/random.h"
#include <assert.h>

#define T Label_t

static int counter = 0;

struct T {
    int count;
    int hashCode;
    Plist_t plist;
};

T Label_new(void) {
    T x;

    Mem_NEW(x);
    x->count = counter++;
    x->hashCode = Random_nextInt();
    x->plist = Plist_new();
    return x;
}

int Label_hashCode(T x) {
    assert(x);
    return x->hashCode;
}

String_t Label_toString(T x) {
    assert(x);
    return String_concat("L_", Int_toString(x->count), 0);
}

int Label_equals(T x, T y) {
    assert(x);
    assert(y);
    return x == y;
}

Plist_t Label_plist(T x) {
    assert(x);
    return x->plist;
}

void Label_print(T x) {
    assert(x);
    printf("%s_%d", "L", x->count);
}

#undef T
