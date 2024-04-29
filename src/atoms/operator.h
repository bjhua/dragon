#ifndef OPERATOR_H
#define OPERATOR_H

#include "../lib/string.h"

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
} Operator_t;

String_t Operator_toString(Operator_t);
Operator_t Operator_fromString(String_t);
long Operator_binary(long, Operator_t, long);
long Operator_unary(Operator_t, long);

#undef T

#endif
