/* Copyright (C) 2011-2012, Baojian Hua.
 * Copyright (C) 2011-2012, School of Software Engineering, USTC.
 *
 * See the Copyright for more information.
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "../lib/string.h"
#include "../control/coordinate.h"
#include "../control/region.h"

typedef enum {
    TOKEN_AND = -128,
    TOKEN_BREAK,
    TOKEN_CATCH,
    TOKEN_CLASS,
    TOKEN_CONTINUE,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_EQ,
    TOKEN_FOR,
    TOKEN_GE,
    TOKEN_ID,
    TOKEN_IF,
    TOKEN_INT,
    TOKEN_INTLIT,
    TOKEN_LE,
    TOKEN_NEQ,
    TOKEN_NEW,
    TOKEN_NULL,
    TOKEN_OF,
    TOKEN_OR,
    TOKEN_RETURN,
    TOKEN_STRING,
    TOKEN_STRINGLIT,
    TOKEN_THROW,
    TOKEN_TRY,
    TOKEN_WHILE
} Token_Kind_t;

#define T Token_t

typedef struct T *T;

struct T {
    Token_Kind_t kind;
    String_t lexeme;
    Region_t region;
};

T Token_new(Token_Kind_t kind, String_t lexeme, Coordinate_t left, Coordinate_t right);

String_t Token_Kind_toString(Token_Kind_t kind);

String_t Token_toString(T t);

String_t Token_toStringWithPos(T t);

#undef T

#endif
