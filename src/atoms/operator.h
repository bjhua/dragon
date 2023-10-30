#ifndef OPERATOR_H
#define OPERATOR_H

#include "../lib/string.h"

#define T Operator_t

typedef enum {
    OP_ADD = 0x0,
    OP_SUB,
    OP_TIMES,
    OP_DIVIDE,
    OP_MODUS,
    OP_OR,
    OP_AND,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_NOT,
    OP_NEG = 14
} T;

String_t Operator_toString(T);

int Operator_binary(int, T, int);

int Operator_unary(T, int);

#undef T

#endif
