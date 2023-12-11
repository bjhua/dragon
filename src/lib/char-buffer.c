#include "char-buffer.h"
#include "mem.h"
#include <assert.h>

#define T CharBuffer_t

#define INIT_SIZE 16
#define SCALE 2

struct T {
    char *buffer;
    long last;
    long size;
};

T CharBuffer_new(void) {
    T x;

    Mem_NEW(x);
    Mem_NEW_SIZE(x->buffer, INIT_SIZE);
    x->last = 0;
    x->size = INIT_SIZE;
    return x;
}

long CharBuffer_numItems(T x) {
    assert(x);
    return x->last;
}

void CharBuffer_resetIndex(T x) {
    assert(x);
    x->last = 0;
}

void CharBuffer_append(T x, int c) {
    char *tmp;

    assert(x);
    if (x->last >= x->size - 1) {
        tmp = x->buffer;
        Mem_NEW_SIZE(x->buffer, x->size * SCALE);
        x->size *= SCALE;
        for (long i = 0; i < x->last; i++)
            x->buffer[i] = tmp[i];
    }
    x->buffer[x->last++] = (char) c;
    //    return;
}

void CharBuffer_appendString(T x, String_t s) {
    assert(x);
    assert(s);
    while (*s) {
        CharBuffer_append(x, *s);
        s++;
    }
    //    return;
}


String_t CharBuffer_toString(T x) {
    assert(x);
    x->buffer[x->last] = '\0';
    return x->buffer;
}

String_t CharBuffer_toStringBeforeClear(T x) {
    String_t str = CharBuffer_toString(x);
    Mem_NEW_SIZE(x->buffer, INIT_SIZE);
    x->last = 0;
    x->size = INIT_SIZE;
    return str;
}

#undef T
