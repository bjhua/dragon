/* Copyright (C).
 *
 */
#include <string.h>
#include "../lib/string.h"
#include "../lib/error.h"
#include "../lib/assert.h"
#include "../lib/char-buffer.h"
#include "../lib/char.h"
#include "../lib/trace.h"
#include "../lib/int.h"
#include "../control/error-msg.h"
#include "buffer.h"
#include "token.h"
#include "lex.h"

#include "keyword.h"

// data structure for position information.
struct Pos_t {
    String_t fname;  // file name
    int line;        // line number, starting from 1
    int column;      // column number, starting from 0
};

static struct Pos_t pos = {0, 1, 0};

static CharBuffer_t numBuffer = 0;
static CharBuffer_t strBuffer = 0;
static CharBuffer_t idBuffer = 0;

static void cookComment();

static Token_t cookId(int theChar, Coordinate_t left);

static Token_t cookNum(int theChar, Coordinate_t left);

static Token_t cookString(Coordinate_t left);

static int eatBlanks();

static void error(char *msg);

static int escape(int c);

static Coordinate_t getPos();

static int isAlpha(int c);

static int columnBackup = 0;

static Coordinate_t getPos() {
    return Coordinate_new(pos.fname, pos.line, pos.column - 1);
}

static void error(char *msg) {
    ErrorMsg_lexError(msg, pos.fname, pos.line, pos.column);
}

static void error2(String_t msg, String_t fname, int line, int column) {
    ErrorMsg_lexError(msg, fname, line, column);
}

static int Lex_getChar() {
    pos.column++;
    return Buffer_getChar();
}

static void Lex_putChar() {
    pos.column--;
    Buffer_putChar();
}

static int eatBlanks() {
    int cur;

    cur = Lex_getChar();
    while (Char_isBlank(cur)) {
        cur = Lex_getChar();
    }
    return cur;
}

static void cookComment() {
    int c;
    c = Lex_getChar();
    while ('\n' != c)
        c = Lex_getChar();
    pos.line++;
    pos.column = 0;
    return;
}

static Token_t cookNum(int c, Coordinate_t leftPos) {
    CharBuffer_append(numBuffer, c);
    c = Lex_getChar();
    while (Char_isDigit(c)) {
        CharBuffer_append(numBuffer, c);
        c = Lex_getChar();
    }
    Lex_putChar();
    return Token_new(TOKEN_INTLIT, CharBuffer_toStringBeforeClear
            (numBuffer), leftPos, getPos());
}

static Token_t cookString(Coordinate_t leftPos) {
    int c;
    String_t fname = pos.fname;
    int line = pos.line;
    int column = pos.column;

    c = Lex_getChar();
    while (c != EOF
           && c != '\n'
           && c != '\"') {
        if (c == '\\') {
            c = Lex_getChar();
            CharBuffer_append(strBuffer, escape(c));
        } else CharBuffer_append(strBuffer, c);
        c = Lex_getChar();
    }
    if (c == EOF)
        error2("unclosed string", fname, line, column);
    else if (c == '\n')
        error("don't allow newLine in strings");
    return Token_new(TOKEN_STRINGLIT, CharBuffer_toStringBeforeClear
            (strBuffer), leftPos, getPos());
}

static int escape(int c) {
    switch (c) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case '\"':
            return '\"';
        default:
            ErrorMsg_lexError("bad escape sequence", pos.fname, pos.line, pos.column);
            return 0;
    }
}

static int isAlpha(int c) {
    return (Char_isAlpha(c))
           || ('_' == c);
}

static Token_t cookId(int c, Coordinate_t leftPos) {
    Token_Kind_t kind;
    String_t str;

    CharBuffer_append(idBuffer, c);
    c = Lex_getChar();
    while (isAlpha(c) || Char_isDigit(c)) {
        CharBuffer_append(idBuffer, c);
        c = Lex_getChar();
    }
    Lex_putChar();
    str = CharBuffer_toStringBeforeClear(idBuffer);
    kind = isKeyWord(str);
    if (kind > 0)
        return Token_new(kind, 0, leftPos, getPos());
    return Token_new(TOKEN_ID, str, leftPos, getPos());
}


/* This is a diagram-trasition algorithm as described
 * in the section 3.4.1 in the Dragon book.
 */
Token_t Lex_getTokenTraced() {
    // "leftPos" is the starting point of some token.
    Coordinate_t leftPos;
    int firstChar;

    // eat blanks, until we reach the first non-blank char.
    firstChar = eatBlanks();

    // For now, only accept Unix-style newlines, but it's
    // not hard to add support for other styles of newlines
    // such as the Windows '\r\n'.
    if ('\n' == firstChar) {
        pos.line++;
        // We should backup the old column number for ...
        columnBackup = pos.column;
        pos.column = 0;
        // recursively call itself to get a token
        return Lex_getTokenTraced();
    }

    // from now on, ready to handle a normal token, first
    // record the coordinate of the starting point.
    leftPos = getPos();

    // if the first character is a digit, goto "cookNum".
    // the following other cases are similar.
    if (Char_isDigit(firstChar))
        return cookNum(firstChar, leftPos);

    if ('\"' == firstChar)
        return cookString(leftPos);

    if (Char_isAlpha(firstChar))
        return cookId(firstChar, leftPos);

    switch (firstChar) {
        case '/': {
            // we should check a character further.
            int secondChar = Lex_getChar();

            if (secondChar == '/') {
                cookComment();
                return Lex_getTokenTraced();
            }
            Lex_putChar();
            return Token_new(TOKEN_DIVIDE, 0, leftPos, getPos());
        }
        case '<': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '=':
                    return Token_new(TOKEN_LE, 0, leftPos, getPos());
                default:
                    Lex_putChar();
                    return Token_new(TOKEN_LT, 0, leftPos, getPos());
            }
        }
        case '>': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '=':
                    return Token_new(TOKEN_GE, 0, leftPos, getPos());
                default:
                    Lex_putChar();
                    return Token_new(TOKEN_GT, 0, leftPos, getPos());
            }
        }
        case '-':
            return Token_new(TOKEN_MINUS, 0, leftPos, getPos());
        case '=': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '=':
                    return Token_new(TOKEN_EQ, 0, leftPos, getPos());
                default:
                    Lex_putChar();
                    return Token_new(TOKEN_ASSIGN, 0, leftPos, getPos());
            }
        }
        case '!': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '=':
                    return Token_new(TOKEN_NEQ, 0, leftPos, getPos());
                default:
                    Lex_putChar();
                    return Token_new(TOKEN_NOT, 0, leftPos, getPos());
            }
        }
        case '|': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '|':
                    return Token_new(TOKEN_OR, 0, leftPos, getPos());
                default:
                    error("bad operator | ");
            }
        }
        case '&': {
            int secondChar = Lex_getChar();

            switch (secondChar) {
                case '&':
                    return Token_new(TOKEN_AND, 0, leftPos, getPos());
                default:
                    error("bad operator & ");
            }
        }
        case '+':
            return Token_new(TOKEN_ADD, 0, leftPos, getPos());
        case ';':
            return Token_new(TOKEN_SEMI, 0, leftPos, getPos());
        case ',':
            return Token_new(TOKEN_COMMER, 0, leftPos, getPos());
        case '*':
            return Token_new(TOKEN_TIMES, 0, leftPos, getPos());
        case '%':
            return Token_new(TOKEN_PERCENT, 0, leftPos, getPos());
        case '[':
            return Token_new(TOKEN_LBRACK, 0, leftPos, getPos());
        case ']':
            return Token_new(TOKEN_RBRACK, 0, leftPos, getPos());
        case '{':
            return Token_new(TOKEN_LBRACE, 0, leftPos, getPos());
        case '}':
            return Token_new(TOKEN_RBRACE, 0, leftPos, getPos());
        case '(':
            return Token_new(TOKEN_LPAREN, 0, leftPos, getPos());
        case ')':
            return Token_new(TOKEN_RPAREN, 0, leftPos, getPos());
        case '.':
            return Token_new(TOKEN_DOT, 0, leftPos, getPos());
        case EOF:
            return Token_new(TOKEN_EOF, 0, leftPos, leftPos);
        default:
            error(String_concat("illegal character: ", Char_toString(firstChar), 0));
            return 0;
    }
    Error_impossible ();
    return 0;
}


static void Trace_arg() {
    //printf ("%s", "()");
    return;
}

static void Trace_result(Token_t r) {
    fprintf(stdout, "%s", Token_toStringWithPos(r));
}

Token_t Lex_getToken() {
    Token_t r;

    Trace_TRACE ("Lex_getToken", Lex_getTokenTraced, (), Trace_arg, r, Trace_result);
    return r;
}

void Lex_init(String_t fname) {
    Buffer_init(fname);
    pos.fname = fname;
    pos.line = 1;
    pos.column = 0;
    numBuffer = CharBuffer_new();
    strBuffer = CharBuffer_new();
    idBuffer = CharBuffer_new();
    return;
}

