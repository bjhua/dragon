#ifndef APPEND_LIST_H
#define APPEND_LIST_H

#include "list.h"

#define T AppendList_t

typedef struct T *T;

T AppendList_new ();

#undef T

#endif
