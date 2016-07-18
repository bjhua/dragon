#ifndef LEX_H
#define LEX_H

#include "token.h"

Token_t Lex_getToken ();
void Lex_init (String_t fileName);

#endif
