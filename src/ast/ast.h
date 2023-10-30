#ifndef AST_H
#define AST_H

#include "../lib/list.h"
#include "../lib/file.h"
#include "../lib/box.h"
#include "../control/region.h"
#include "../elaborate/type.h"
#include "ast-id.h"

#define Id_t AstId_t

#define B Ast_Block_t
#define C Ast_Class_t
#define D Ast_Dec_t
#define E Ast_Exp_t
#define F Ast_Fun_t
#define L Ast_Lval_t
#define P Ast_Prog_t
#define S Ast_Stm_t
#define T Ast_Type_t

typedef struct B *B;
typedef struct C *C;
typedef struct D *D;
typedef struct E *E;
typedef struct F *F;
typedef struct L *L;
typedef struct P *P;
typedef struct S *S;
typedef struct T *T;

/***************************************************************/
// left-value
struct L {
    enum {
        AST_LVAL_VAR,
        AST_LVAL_DOT,
        AST_LVAL_ARRAY
    } kind;
    union {
        Id_t var;
        struct {
            L lval;
            Id_t var;
        } dot;
        struct {
            L lval;
            E exp;
        } array;
    } u;
    // Type of the whole left value
    // this field will be filled by the elaborator
    Type_t ty;
    Region_t region;
};

L Ast_Lval_new_var(Id_t, Type_t ty, Region_t);

L Ast_Lval_new_dot(L, Id_t, Type_t ty, Region_t);

L Ast_Lval_new_array(L, E, Type_t ty, Region_t);

File_t Ast_Lval_print(File_t, L);

/////////////////////////////////////////////
// expressions.
typedef enum {
    AST_EXP_ASSIGN,
    AST_EXP_ADD,
    AST_EXP_SUB,
    AST_EXP_TIMES,
    AST_EXP_DIVIDE,
    AST_EXP_MODUS,
    AST_EXP_OR,
    AST_EXP_AND,
    AST_EXP_EQ,
    AST_EXP_NE,
    AST_EXP_LT,
    AST_EXP_LE,
    AST_EXP_GT,
    AST_EXP_GE,
    AST_EXP_NOT,
    AST_EXP_NEGATIVE,
    AST_EXP_NULL,
    AST_EXP_INTLIT,
    AST_EXP_STRINGLIT,
    AST_EXP_NEW_CLASS,
    AST_EXP_NEW_ARRAY,
    AST_EXP_LVAL,
    AST_EXP_CALL
} Ast_Exp_Kind_t;

struct E {
    Ast_Exp_Kind_t kind;
    union {
        // assignment is treated specially.
        struct {
            E left;
            E right;
        } assign;
        struct {
            E left;
            E right;
        } bop;
        struct {
            E e;
        } unary;
        struct {
            T type;
            E size;
        } newArray;
        struct {
            AstId_t name;
            /* List_t<E> */
            List_t args;
        } newClass;
        int intlit;
        String_t stringlit;
        struct {
            AstId_t f;
            /* List<E> */
            List_t args;
        } call;
        L lval;
    } u;
    Type_t ty;
    Region_t region;
};

E Ast_Exp_new_assign(E left,
                     E right,
                     Type_t ty,
                     Region_t r);

E Ast_Exp_new_bop(Ast_Exp_Kind_t kind, E left, E right, Type_t ty, Region_t r);

E Ast_Exp_new_unary(Ast_Exp_Kind_t kind,
                    E e, Type_t ty, Region_t r);

E Ast_Exp_new_null();

E Ast_Exp_new_intlit(String_t);

E Ast_Exp_new_stringlit(String_t);

E Ast_Exp_new_newClass(Id_t id, List_t, Type_t ty);
// "type" is source type and is the element type.
// "ty" is semantic type and is the expression type.
E Ast_Exp_new_newArray(T type, Type_t ty, E size);

E Ast_Exp_new_call(AstId_t f, List_t, Type_t ty);

E Ast_Exp_new_lval(L, Type_t ty);

File_t Ast_Exp_print(File_t file, E);

//=================================================
// statement-related
struct S {
    enum {
        AST_STM_EXP,
        AST_STM_IF,
        AST_STM_WHILE,
        AST_STM_DO,
        AST_STM_FOR,
        AST_STM_BREAK,
        AST_STM_CONTINUE,
        AST_STM_THROW,
        AST_STM_TRYCATCH,
        AST_STM_RETURN,
        AST_STM_BLOCK
    } kind;
    union {
        E exp;
        struct {
            E cond;
            S then;
            S elsee; // empty list for dangling else (not 0)
        } iff;
        struct {
            E cond;
            S body;
        } whilee;
        struct {
            E cond;
            S body;
        } doo;
        struct {
            E header;
            E cond;
            E tail;
            S body;
        } forr;
        struct {
            S tryy;
            S catchh;
        } trycatch;
        E returnn;
        B block;
    } u;
    Region_t region;
};

S Ast_Stm_new_exp(E);

S Ast_Stm_new_if(E, S, S, Region_t);

S Ast_Stm_new_while(E, S, Region_t);

S Ast_Stm_new_do(E, S, Region_t);

S Ast_Stm_new_for(E, E, E, S, Region_t);

S Ast_Stm_new_break(Region_t);

S Ast_Stm_new_continue(Region_t);

S Ast_Stm_new_throw(Region_t);

S Ast_Stm_new_tryCatch(S, S, Region_t);

S Ast_Stm_new_return(E, Region_t);

S Ast_Stm_new_block(B);

Box_t Ast_Stm_box(S);

File_t Ast_Stm_print(File_t file, S);

/////////////////////////////////////////////////////
// declaration. for 3 purposes: 
//   * class fields, 
//   * local dec, and
//   * function argument declaration.
//
// There is a subty here: the "init" field is only
// meaningful for local dec. otherwise 0.
struct D {
    T type;
    AstId_t var;
    E init;
};

D Ast_Dec_new(T type, AstId_t var, E init);

File_t Ast_Dec_print(File_t, D);


/////////////////////////////////////////////////////
// block
struct B {
    /* List<Dec_t> */
    List_t decs;
    /* List<Stm_t> */
    List_t stms;
};

B Ast_Block_new(List_t, List_t);

File_t Ast_Block_print(File_t file, B);

//////////////////////////////////////////////////
/* function */
struct F {
    T type;
    Id_t name;
    /* List<Dec_t> */
    List_t args;
    B block;
    Region_t region;
};

F Ast_Fun_new(T type, Id_t name, List_t args, B b, Region_t r);

Box_t Ast_Fun_box(F);

File_t Ast_Fun_print(File_t file, F);

////////////////////////////////////////////////////
/* type definition */
struct T {
    enum {
        AST_TYPE_INT,
        AST_TYPE_STRING,
        AST_TYPE_ID
    } kind;
    Id_t id;       // effective only for AST_TYPE_ID
    int isArray;   // 1 for array, otherwise 0.
};

T Ast_Type_new_int();

T Ast_Type_new_string();

T Ast_Type_new_id(Id_t id);

// set the "isArray" field
void Ast_Type_setArray(T);

File_t Ast_Type_print(File_t, T);

Box_t Ast_Type_box(T);

String_t Ast_Type_toString(T);

///////////////////////////////////////////////
// class definition
struct C {
    AstId_t name;
    // List_t<Ast_Dec_t>
    List_t fields;
};

C Ast_Class_new(AstId_t, List_t);

Box_t Ast_Class_box(C);

File_t Ast_Class_print(File_t, C);

////////////////////////////////////////////
// program definition
struct P {
    /* List<Class> */
    List_t classes;
    /* List<F> */
    List_t funcs;
};

P Ast_Prog_new(List_t, List_t);

Box_t Ast_Prog_box(P x);

void Ast_Prog_print(File_t file, P x);

#undef B
#undef C
#undef D
#undef E
#undef F
#undef L
#undef P
#undef S
#undef T

#undef Id_t

#endif

