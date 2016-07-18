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

typedef enum{
  TOKEN_ZERO = 0,
  TOKEN_ADD,
  TOKEN_AND,  
  TOKEN_ASSIGN,
  TOKEN_BREAK,
  TOKEN_CATCH,
  TOKEN_CLASS,
  TOKEN_COMMER,
  TOKEN_CONTINUE,
  TOKEN_DIVIDE,
  TOKEN_DO,
  TOKEN_DOT,
  TOKEN_ELSE,
  TOKEN_EQ,
  TOKEN_EOF,
  TOKEN_FOR,
  TOKEN_GE,
  TOKEN_GT,
  TOKEN_ID,
  TOKEN_IF,
  TOKEN_INT,
  TOKEN_INTLIT,
  TOKEN_LBRACE,
  TOKEN_LBRACK,
  TOKEN_LE,
  TOKEN_LPAREN,
  TOKEN_LT,
  TOKEN_MINUS,
  TOKEN_NEQ,
  TOKEN_NEW,
  TOKEN_NOT,
  TOKEN_NULL,
  TOKEN_OF,
  TOKEN_OR,
  TOKEN_PERCENT,
  TOKEN_RBRACE,
  TOKEN_RBRACK,
  TOKEN_RETURN,
  TOKEN_RPAREN,
  TOKEN_SEMI,
  TOKEN_STRING,
  TOKEN_STRINGLIT,
  TOKEN_TIMES,
  TOKEN_THROW,
  TOKEN_TRY,
  TOKEN_WHILE
}Token_Kind_t;

#define T Token_t

typedef struct T *T;

struct T
{
  Token_Kind_t kind;
  String_t lexeme;
  Region_t region;
};

T Token_new (Token_Kind_t kind
             , String_t lexeme
             , Coordinate_t left
             , Coordinate_t right);
String_t Token_Kind_toString (Token_Kind_t kind);
String_t Token_toString (T t);
String_t Token_toStringWithPos (T t);

#undef T

#endif
