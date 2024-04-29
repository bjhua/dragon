#include "stack.h"
#include "error.h"
#include "mem.h"
#include "string.h"
#include "todo.h"
#include "unused.h"
#include <assert.h>
#include <stdio.h>

#define T Stack_t
#define P Poly_t

T Stack_new(void) {
    return List_new();
}

int Stack_isEmpty(T stk) {
    assert(stk);
    return List_isEmpty(stk);
}

void Stack_push(T stk, P x) {
    assert(stk);
    List_insertFirst(stk, x);
    return;
}

P Stack_pop(T stk) {
    T t;

    assert(stk);
    if (List_isEmpty(stk))
        Error_error("try to pop on empty stacks\n");
    t = stk->next->data;
    stk->next = stk->next->next;
    return t;
}

int Stack_size(T stk) {
    return List_size(stk);
}

T Stack_stack(P x1, ...) {
    UNUSED(x1);
    TODO;
}

char *Stack_toString(T stk, String_t sep,
                     Poly_tyToString tx) {
    T temp = stk;
    String_t str = String_new("[");
    while (temp) {
        str = String_concat(str,
                            tx(temp->data),
                            sep,
                            0);
        temp = temp->next;
    }
    str = String_concat(str, "]", 0);
    return str;
}

P Stack_getTop(T stk) {
    assert(stk);
    if (List_isEmpty(stk)) {
        *((int *) 1) = 0;
        Error_error("try to get top of an empty stack");
    }
    return stk->next->data;
}

#undef T
#undef P
