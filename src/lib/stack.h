#ifndef STACK_H
#define STACK_H

#include "list.h"
#include "poly.h"

#define T Stack_t
#define P Poly_t

typedef List_t Stack_t;

T Stack_new(void);

void Stack_push(T stk, P x);

P Stack_pop(T stk);

int Stack_isEmpty(T stk);

int Stack_size(T stk);

T Stack_stack(P x1, ...);

char *Stack_toString(T stk, String_t sep,
                     Poly_tyToString tx);

P Stack_getTop(T stk);

#undef P
#undef T

#endif
