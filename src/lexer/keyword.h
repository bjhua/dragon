#ifndef KEY_WORD_H
#define KEY_WORD_H

#include "../lib/string.h"
#include "token.h"

struct KeyWord_t {
    char *str;
    Token_Kind_t kind;
};

struct KeyWord_t keyWordTable[] = {
        {"break", TOKEN_BREAK},
        {"catch", TOKEN_CATCH},
        {"class", TOKEN_CLASS},
        {"continue", TOKEN_CONTINUE},
        {"do", TOKEN_DO},
        {"else", TOKEN_ELSE},
        {"for", TOKEN_FOR},
        {"if", TOKEN_IF},
        {"int", TOKEN_INT},
        {"new", TOKEN_NEW},
        {"null", TOKEN_NULL},
        {"return", TOKEN_RETURN},
        {"string", TOKEN_STRING},
        {"throw", TOKEN_THROW},
        {"try", TOKEN_TRY},
        {"while", TOKEN_WHILE},
        {0, (Token_Kind_t) 0}};

static int isKeyWord(const char *s) {
    char *t;
    for (int i = 0; (t = keyWordTable[i].str); i++)
        if (0 == strcmp(s, t))
            return keyWordTable[i].kind;
    return 0;
}

#endif
