#include "../lib/mem.h"
#include "../lib/assert.h"
#include "token.h"

#define T Token_t

T Token_new(Token_Kind_t kind, String_t lexeme, Coordinate_t left, Coordinate_t right) {
    Token_t temp;

    Mem_NEW(temp);
    temp->kind = kind;
    temp->lexeme = lexeme;
    temp->region = Region_new(left, right);
    return temp;
}

String_t Token_Kind_toString(Token_Kind_t kind) {
    switch (kind) {
        case TOKEN_ADD:
            return "+";
        case TOKEN_AND:
            return "&&";
        case TOKEN_ASSIGN:
            return "=";
        case TOKEN_BREAK:
            return "break";
        case TOKEN_CATCH:
            return "catch";
        case TOKEN_CLASS:
            return "class";
        case TOKEN_COMMER:
            return ",";
        case TOKEN_CONTINUE:
            return "cotinue";
        case TOKEN_DIVIDE:
            return "/";
        case TOKEN_DO:
            return "do";
        case TOKEN_DOT:
            return ".";
        case TOKEN_ELSE:
            return "else";
        case TOKEN_EQ:
            return "==";
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_FOR:
            return "for";
        case TOKEN_GE:
            return ">=";
        case TOKEN_GT:
            return ">";
        case TOKEN_ID:
            return "id<>";
        case TOKEN_IF:
            return "if";
        case TOKEN_INT:
            return "int";
        case TOKEN_INTLIT:
            return "int<>";
        case TOKEN_LBRACE:
            return "{";
        case TOKEN_LBRACK:
            return "[";
        case TOKEN_LE:
            return "<=";
        case TOKEN_LPAREN:
            return "(";
        case TOKEN_LT:
            return "<";
        case TOKEN_MINUS:
            return "-";
        case TOKEN_NEQ:
            return "!=";
        case TOKEN_NEW:
            return "new";
        case TOKEN_NOT:
            return "!";
        case TOKEN_NULL:
            return "null";
        case TOKEN_OF:
            return "of";
        case TOKEN_OR:
            return "||";
        case TOKEN_PERCENT:
            return "%";
        case TOKEN_RBRACE:
            return "}";
        case TOKEN_RBRACK:
            return "]";
        case TOKEN_RETURN:
            return "return";
        case TOKEN_RPAREN:
            return ")";
        case TOKEN_SEMI:
            return ";";
        case TOKEN_STRING:
            return "string";
        case TOKEN_STRINGLIT:
            return "str<>";
        case TOKEN_TIMES:
            return "*";
        case TOKEN_THROW:
            return "throw";
        case TOKEN_TRY:
            return "try";
        case TOKEN_WHILE:
            return "while";
        default:
            fprintf(stderr, "kind==%d\n", kind);
            Error_impossible ();
            return 0;
    }
    Error_impossible ();
    return 0;
}

String_t Token_toString(T t) {
    String_t extra = "";

    Assert_ASSERT(t);
    switch (t->kind) {
        case TOKEN_ID:
        case TOKEN_STRINGLIT:
        case TOKEN_INTLIT:
            extra = t->lexeme;
            break;
        default:
            break;
    }
    return String_concat(Token_Kind_toString(t->kind), extra, 0);
}

String_t Token_toStringWithPos(T t) {
    String_t extra = "";

    switch (t->kind) {
        case TOKEN_ID:
        case TOKEN_STRINGLIT:
        case TOKEN_INTLIT:
            extra = t->lexeme;
            break;
        default:
            break;
    }
    return String_concat
            (Token_Kind_toString(t->kind), extra, " \t\tat: ", Coordinate_toString(Region_from(t->region)),
             0);
}


#undef T
