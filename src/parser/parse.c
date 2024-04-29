#include "parse.h"
#include "../control/error-msg.h"
#include "../lexer/lex.h"
#include "../lexer/token.h"
#include "../lib/dlist.h"
#include "../lib/error.h"
#include "../lib/hash.h"
#include "../lib/string.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"
#include <assert.h>

static Token_t current;
/* tyTable: String_t -> 1 */
static Hash_t tyTable = 0;

//static long lookupType(String_t name) {
//    return (long) Hash_lookup(tyTable, name);
//}

static void advance(void);
static Token_t eatToken(Token_Kind_t kind);
static String_t convertOperator(char kind);
static Ast_Type_t Parse_betaType(void);
static Ast_Type_t Parse_type(void);
static Ast_Lval_t Parse_lval(Token_t first);
static E Parse_exp(void);
static Ast_Stm_t Parse_stm(void);
static Ast_Block_t Parse_block(void);
static void error(String_t msg, Region_t r) {
    ErrorMsg_syntaxError(msg, r);
}

static Region_t region = 0;

static void error_dupTypeName(String_t old, String_t cur) {
    error(String_concat("type name redefined: ", cur, "the previous one   : ", old, 0), region);
}

static void advance(void) {
    current = Lex_getToken();
}


static Token_t eatToken(int kind) {
    Token_t token;

    token = current;
    if (current->kind != kind)
        error(String_concat("expects: ", Token_Kind_toString(kind), "\nbut got: ", Token_toString(current), 0),
              current->region);
    region = current->region;
    advance();
    return token;
}

static AstId_t convertToken(Token_t t) {
    assert(t->kind == TOKEN_ID);
    return AstId_fromString(t->lexeme, t->region);
}


static String_t convertOperator(char kind) {
    switch (kind) {
        case '+':
            return "+";
        case '-':
            return "-";
        default:
            Error_impossible();
            return 0;
    }
}


static List_t Parse_parameters(void) {
    List_t list = List_new();
    E e;

    if (current->kind == ')')
        return list;
    e = Parse_exp();
    List_insertLast(list, e);
    while (current->kind == ',') {
        advance();
        e = Parse_exp();
        List_insertLast(list, e);
    }
    return list;
}

static E Parse_lvalOrCall(void) {
    Token_t s;
    AstId_t id;

    s = eatToken(TOKEN_ID);
    id = convertToken(s);
    switch ((int) current->kind) {
        case '[':
        case '.': {
            Ast_Lval_t lval;

            lval = Parse_lval(s);
            return Ast_Exp_new_lval(lval, 0);
        }
        case '(': {
            List_t args = List_new();
            advance();
            args = Parse_parameters();
            eatToken(')');
            return Ast_Exp_new_call(id, args, 0);
        }
        default: {
            Ast_Lval_t lval = Ast_Lval_new_var(id, 0, s->region);
            return Ast_Exp_new_lval(lval, 0);
        }
    }
    Error_impossible();
    return 0;
}

// new \beta (e, e, ..., e)
// new \beta [e]
// "new" has been eaten.
static E Parse_classOrArray(void) {
    AstId_t id;

    Ast_Type_t t = Parse_betaType();
    switch ((int) current->kind) {
        case '[': {
            E e;
            advance();
            e = Parse_exp();
            eatToken(']');
            return Ast_Exp_new_newArray(t, 0, e);
        }
        case '(': {
            List_t fields = List_new();
            // type should only be a class name
            if (t->kind != AST_TYPE_ID) {
                error(String_concat("expects: class name"
                                    "but got: ",
                                    Ast_Type_toString(t), 0),
                      current->region);
                return 0;
            }
            id = t->id;
            advance();
            fields = Parse_parameters();
            eatToken(')');
            return Ast_Exp_new_newClass(id, fields, 0);
        }
        default: {
            error(String_concat("expects: { or [\n"
                                "but got: ",
                                Token_toString(current), 0),
                  current->region);
            return 0;
        }
    }
    Error_impossible();
    return 0;
}

//
static E Parse_factor(void) {
    E e;
    String_t s;

    switch ((int) current->kind) {
        case '(':
            advance();
            e = Parse_exp();
            eatToken(')');
            return e;
        case TOKEN_INTLIT:
            s = current->lexeme;
            advance();
            return Ast_Exp_new_intlit(s);
        case TOKEN_NEW: {
            advance();
            return Parse_classOrArray();
        }
        case TOKEN_ID: {
            return Parse_lvalOrCall();
        }
        case TOKEN_STRINGLIT:
            s = current->lexeme;
            advance();
            return Ast_Exp_new_stringlit(s);
        case TOKEN_NULL:
            advance();
            return Ast_Exp_new_null();
        default:
            error(String_concat("expects: (, integer, identifier, "
                                "string, null\n"
                                "but got: ",
                                Token_Kind_toString(current->kind), 0),
                  current->region);
            return (Poly_t) 0;
    }
    Error_impossible();
}

static E Parse_unary(void) {
    E e;
    List_t all = List_new();

    while (current->kind == '-' || current->kind == '!') {
        Region_t r = current->region;
        List_insertFirst(all,
                         Tuple_new((Poly_t) current->kind, r));
        advance();
    }
    e = Parse_factor();
    all = List_getFirst(all);
    while (all) {
        Tuple_t tuple = (Tuple_t) all->data;
        long kind = (long) (Tuple_first(tuple));
        Region_t r = Tuple_second(tuple);
        if (kind == '-') {
            e = Ast_Exp_new_unary(AST_EXP_NEGATIVE, e, 0, r);
        } else if (kind == '!')
            e = Ast_Exp_new_unary(AST_EXP_NOT, e, 0, r);
        else
            Error_bug("impossible");
        all = all->next;
    }
    return e;
}

static E Parse_term(void) {
    E e1, e2;
    char kind;

    e1 = Parse_unary();
    while (current->kind == '*' || current->kind == '/' || current->kind == '%') {
        Region_t r = current->region;
        kind = (char) current->kind;
        advance();
        e2 = Parse_unary();
        e1 = Ast_Exp_new_bop(convertOperator(kind), e1, e2, 0, r);
    }
    return e1;
}

static E Parse_expr(void) {
    E e1, e2;
    char kind;

    e1 = Parse_term();
    while (current->kind == '+' || current->kind == '-') {
        Region_t r = current->region;
        kind = (char) current->kind;
        advance();
        e2 = Parse_term();
        e1 = Ast_Exp_new_bop(convertOperator(kind), e1, e2, 0, r);
    }
    return e1;
}


static E Parse_rel(void) {
    E e1, e2;
    char kind;

    e1 = Parse_expr();
    while (current->kind == '<' || current->kind == TOKEN_LE || current->kind == TOKEN_GE || current->kind == '>') {
        Region_t r = current->region;
        kind = (char) current->kind;
        advance();
        e2 = Parse_expr();
        e1 = Ast_Exp_new_bop(convertOperator(kind), e1, e2, 0, r);
    }
    return e1;
}


static E Parse_equality(void) {
    E e1, e2;
    char kind;

    e1 = Parse_rel();
    while (current->kind == TOKEN_EQ || current->kind == TOKEN_NEQ) {
        Region_t r = current->region;
        kind = (char) current->kind;
        advance();
        e2 = Parse_rel();
        e1 = Ast_Exp_new_bop(convertOperator(kind), e1, e2, 0, r);
    }
    return e1;
}

static E Parse_join(void) {
    E e1, e2;

    e1 = Parse_equality();
    while (current->kind == TOKEN_AND) {
        Region_t r = current->region;
        advance();
        e2 = Parse_equality();
        e1 = Ast_Exp_new_bop("&&", e1, e2, 0, r);
    }
    return e1;
}


static E Parse_bool(void) {
    E e1, e2;

    e1 = Parse_join();
    while (current->kind == TOKEN_OR) {
        Region_t r = current->region;
        advance();
        e2 = Parse_join();
        e1 = Ast_Exp_new_bop("||", e1, e2, 0, r);
    }
    return e1;
}

// e -> be = be = be = ... = be
//   -> be
static E Parse_assign(void) {
    Dlist_t dlist = Dlist_new();
    E e1, e2;
    int flag = 0;

    e1 = Parse_bool();
    Dlist_insertLast(dlist, Tuple_new(e1,
                                      current->region));
    while (current->kind == '=') {
        flag = 1;
        Region_t r = current->region;
        advance();
        e1 = Parse_bool();
        Dlist_insertLast(dlist, Tuple_new(e1, r));
    }
    if (!flag)
        return e1;

    // reverse scan the list, for the "=" is
    // right-associative. That is:
    //   e1 = e2 = e3
    // is interpreted as:
    //   e1 = (e2 = e3).
    Dlist_t tmp = dlist->prev;
    e2 = Tuple_first(tmp->data);
    tmp = tmp->prev;
    while (tmp != dlist) {
        e2 = Ast_Exp_new_assign(Tuple_first(tmp->data), e2, 0, Tuple_second(tmp->data));
        tmp = tmp->prev;
    }
    return e2;
}

static E Parse_exp(void) {
    return Parse_assign();
}

//////////////////////////////////////////
//     lval -> id
//          -> lval.x
//          -> lval[e]
static Ast_Lval_t Parse_lval(Token_t first) {
    Ast_Lval_t lval;
    AstId_t id;
    E exp;
    Region_t r;

    if (!first) {
        first = eatToken(TOKEN_ID);
    }
    r = first->region;
    id = convertToken(first);
    lval = Ast_Lval_new_var(id, 0, r);
    for (;;) {
        switch ((int) current->kind) {
            case '.':
                advance();
                first = eatToken(TOKEN_ID);
                id = convertToken(first);
                lval = Ast_Lval_new_dot(lval, id, 0, r);
                break;
            case '[':
                advance();
                exp = Parse_exp();
                eatToken(']');
                lval = Ast_Lval_new_array(lval, exp, 0, r);
                break;
            default:
                return lval;
        }
    }
    Error_bug("impossible");
    return 0;
}

/////////////////////////////////////////////
//   for (e; e; e)
//     s
static Ast_Stm_t Parse_for(void) {
    E init, cond, tail;
    Ast_Stm_t body;
    Token_t t;

    advance();
    t = eatToken('(');
    init = Parse_exp();
    eatToken(';');
    cond = Parse_exp();
    eatToken(';');
    tail = Parse_exp();
    eatToken(')');
    body = Parse_stm();
    return Ast_Stm_new_for(init, cond, tail, body, t->region);
}

static Ast_Stm_t Parse_do(void) {
    E condition;
    Ast_Stm_t body;
    Token_t t;

    advance();
    body = Parse_stm();
    t = eatToken(TOKEN_WHILE);
    eatToken('(');
    condition = Parse_exp();
    eatToken(')');
    eatToken(';');
    return Ast_Stm_new_do(condition, body, t->region);
}

static Ast_Stm_t Parse_while(void) {
    E condition;
    Ast_Stm_t body;
    Token_t t;

    advance();
    t = eatToken('(');
    condition = Parse_exp();
    eatToken(')');
    body = Parse_stm();
    return Ast_Stm_new_while(condition, body, t->region);
}

static Ast_Stm_t Parse_if(void) {
    E condition;
    Ast_Stm_t then, elsee =
                            Ast_Stm_new_block(Ast_Block_new(List_new(), List_new()));
    Token_t t;

    advance();
    t = eatToken('(');
    condition = Parse_exp();
    eatToken(')');
    then = Parse_stm();
    if (current->kind == TOKEN_ELSE) {
        advance();
        elsee = Parse_stm();
    }
    return Ast_Stm_new_if(condition, then, elsee, t->region);
}

static Ast_Stm_t Parse_stm(void) {
    E e;
    Ast_Block_t b;

    switch ((int) current->kind) {
        case TOKEN_IF:
            return Parse_if();
        case TOKEN_WHILE:
            return Parse_while();
        case TOKEN_DO:
            return Parse_do();
        case TOKEN_FOR:
            return Parse_for();
        case TOKEN_BREAK: {
            Region_t r = current->region;
            advance();
            eatToken(';');
            return Ast_Stm_new_break(r);
        }
        case TOKEN_CONTINUE: {
            Region_t r = current->region;
            advance();
            eatToken(';');
            return Ast_Stm_new_continue(r);
        }
        case TOKEN_RETURN: {
            Region_t r = current->region;
            advance();
            e = Parse_exp();
            eatToken(';');
            return Ast_Stm_new_return(e, r);
        }
        case TOKEN_THROW: {
            Region_t r = current->region;
            advance();
            eatToken(';');
            return Ast_Stm_new_throw(r);
        }
        case TOKEN_TRY: {
            Ast_Stm_t tryBlock, catchBlock;
            Region_t r;
            r = current->region;
            advance();
            tryBlock = Parse_stm();
            eatToken(TOKEN_CATCH);
            catchBlock = Parse_stm();
            return Ast_Stm_new_tryCatch(tryBlock, catchBlock, r);
        }
        case '{': {
            b = Parse_block();
            return Ast_Stm_new_block(b);
        }
        default: {
            E ee;

            ee = Parse_exp();
            eatToken(';');
            return Ast_Stm_new_exp(ee);
        }
    }
    Error_impossible();
    return (Poly_t) 0;
}

static List_t Parse_stmList(void) {
    List_t list = List_new();
    Ast_Stm_t s;

    while (current->kind != '{') {
        s = Parse_stm();
        List_insertLast(list, s);
    }
    return list;
}

// \beta -> int | string | id
static Ast_Type_t Parse_betaType(void) {
    switch (current->kind) {
        default: {
            error(String_concat("expects: int, string, or id<>"
                                "but got: ",
                                Token_toString(current), 0),
                  current->region);
            return (void *) 0;
        }
        case TOKEN_INT: {
            advance();
            return Ast_Type_new_int();
        }
        case TOKEN_STRING: {
            advance();
            return Ast_Type_new_string();
        }
        case TOKEN_ID: {
            AstId_t id = convertToken(eatToken(TOKEN_ID));
            return Ast_Type_new_id(id);
        }
    }
    //    return (void *) 0;
}

// t -> \beta | \beta[]
static Ast_Type_t Parse_type(void) {
    Ast_Type_t t;

    t = Parse_betaType();
    if (current->kind == '[') {
        Ast_Type_setArray(t);
        advance();
        eatToken(']');
    }
    return t;
}

static List_t Parse_decList(void) {
    List_t list = List_new();
    Ast_Type_t type;
    Token_t name;
    AstId_t id;
    E init = 0;

    while (current->kind == TOKEN_INT || current->kind == TOKEN_STRING || current->kind == TOKEN_ID) {
        if (current->kind == TOKEN_ID && !Hash_lookup(tyTable, current->lexeme))
            return list;

        type = Parse_type();
        name = eatToken(TOKEN_ID);
        id = convertToken(name);
        if (current->kind == '=') {
            advance();
            init = Parse_exp();
        }
        eatToken(';');
        List_insertLast(list, Ast_Dec_new(type, id, init));
        init = 0;
    }
    return list;
}

static Ast_Block_t Parse_block(void) {
    List_t decs, stms;
    Ast_Block_t b;

    eatToken('{');
    decs = Parse_decList();
    stms = Parse_stmList();
    eatToken('}');
    b = Ast_Block_new(decs, stms);
    return b;
}

static List_t Parse_arguments(void) {
    List_t list = List_new();
    Ast_Type_t type;
    Token_t name;
    AstId_t id;

    if (current->kind == '(')
        return list;

    type = Parse_type();
    name = eatToken(TOKEN_ID);
    id = convertToken(name);
    List_insertLast(list, Ast_Dec_new(type, id, 0));
    while (current->kind == ',') {
        advance();
        type = Parse_type();
        name = eatToken(TOKEN_ID);
        id = convertToken(name);
        List_insertLast(list, Ast_Dec_new(type, id, 0));
    }
    return list;
}

static List_t Parse_functionList(void) {
    List_t list = List_new(), paras;
    Ast_Type_t type;
    Token_t name;
    AstId_t fid;
    Ast_Fun_t f;
    Ast_Block_t b;
    Region_t r;

    while (current->kind != 0) {
        r = current->region;
        type = Parse_type();
        name = eatToken(TOKEN_ID);
        fid = convertToken(name);
        eatToken('(');
        paras = Parse_arguments();
        eatToken(')');
        b = Parse_block();
        f = Ast_Fun_new(type, fid, paras, b, r);
        List_insertLast(list, f);
    }
    return list;
}

//static List_t Parse_classFieldList() {
//    Ast_Type_t type;
//    AstId_t id;
//    List_t fs = List_new();
//
//    while (current->kind != '}') {
//        type = Parse_type();
//        id = convertToken(eatToken(TOKEN_ID));
//        eatToken(';');
//        List_insertLast(fs, Ast_Dec_new(type, id, 0));
//    }
//    return fs;
//}

static List_t Parse_classList(void) {
    Token_t className;
    AstId_t id;
    List_t list = List_new();
    List_t fields;

    while (current->kind == TOKEN_CLASS) {
        advance();
        className = eatToken(TOKEN_ID);
        Hash_insert(tyTable, className->lexeme, (Poly_t) 1);
        id = convertToken(className);
        eatToken('{');

        fields = Parse_decList();

        eatToken('}');
        List_insertLast(list, Ast_Class_new(id, fields));
    }
    return list;
}

static Ast_Prog_t Parse_parseTraced(String_t file) {
    List_t classes, funcs;

    Lex_init(file);
    Id_init();
    // get the first token
    current = Lex_getToken();
    tyTable = Hash_new((tyHashCode) String_hashCode, (Poly_tyEquals) String_equals, (tyDup) error_dupTypeName);

    classes = Parse_classList();
    funcs = Parse_functionList();
    return Ast_Prog_new(classes, funcs);
}

static void Trace_arg(String_t file) {
    fprintf(stdout, "%s", file);
}

static void Trace_result(Ast_Prog_t p) {
    File_saveToFile("parse.result", (Poly_tyPrint) Ast_Prog_print, p);
}

Ast_Prog_t Parse_parse(String_t file) {
    Ast_Prog_t r;

    Trace_TRACE("Parse_parse", Parse_parseTraced, (file), Trace_arg, r, Trace_result);
    return r;
}
