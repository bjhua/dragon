#ifndef HIL_H
#define HIL_H

#include "../lib/list.h"
#include "../lib/file.h"
#include "../atoms/atoms.h"

#define E Hil_Exp_t
#define F Hil_Fun_t
#define L Hil_Lval_t
#define P Hil_Prog_t
#define S Hil_Stm_t

typedef struct E *E;
typedef struct F *F;
typedef struct L *L;
typedef struct P *P;
typedef struct S *S;

/***************************************************************/
/* left-value */
struct L
{
  enum {
    HIL_LVAL_VAR,
    HIL_LVAL_DOT,
    HIL_LVAL_ARRAY
  }kind;
  union{
    Id_t var;
    struct{
      L lval;
      Id_t var;
    }dot;
    struct {
      L lval;
      E exp;
    }array;
  }u;
  Atype_t ty;
};

L Hil_Lval_new_var (Id_t, Atype_t ty);
L Hil_Lval_new_dot (L, Id_t, Atype_t ty);
L Hil_Lval_new_array (L, E, Atype_t ty);
File_t Hil_Lval_print (File_t file, L);

//////////////////////////////////////////////////////
/* expressions */
typedef enum {
  HIL_EXP_BOP,
  HIL_EXP_UOP,
  HIL_EXP_INTLIT,
  HIL_EXP_STRINGLIT,
  HIL_EXP_NEW_CLASS,
  HIL_EXP_NEW_ARRAY,
  HIL_EXP_LVAL,
  HIL_EXP_CALL
} Hil_Exp_Kind_t ;

struct E
{
  Hil_Exp_Kind_t kind;
  union{
    struct {
      E left;
      E right;
      Operator_t op;
    }bop;
    struct {
      E e;
      Operator_t op;
    }unary;
    struct {
      // type of array element
      Atype_t type;
      E size;
    }newArray;
    struct {
      Id_t name;
      // List<E>
      List_t args;
    }newClass;
    int intlit;
    String_t stringlit;
    struct{
      Id_t f;
      /* List<E> */
      List_t args;
      // For function that may throw to a local handler, the 
      // "leave" field is the label for that handler; or else
      // the "leave" label is 0.
      // "normal" field is always the next normal statement
      // after the function call.
      Label_t leave;
      Label_t normal;
    }call;
    L lval;
  }u;
  Atype_t ty;
};

E Hil_Exp_new_bop (Operator_t op,
                   E left,
                   E right,
                   Atype_t ty);
E Hil_Exp_new_unary (Operator_t op,
                     E e,
                     Atype_t ty);
E Hil_Exp_new_intlit (int, Atype_t ty);
E Hil_Exp_new_stringlit (String_t s, Atype_t ty);
E Hil_Exp_new_newArray (Atype_t ty, E size);
E Hil_Exp_new_newClass (Id_t name, List_t args);
E Hil_Exp_new_call (Id_t, List_t, Atype_t
                    , Label_t leave
                    , Label_t normal);
E Hil_Exp_new_lval (L, Atype_t ty);
File_t Hil_Exp_print (File_t file, E);

//////////////////////////////////////////////////////
/* statements */
struct S
{
  enum {
    HIL_STM_ASSIGN,
    HIL_STM_DO,
    HIL_STM_EXP,
    HIL_STM_IF,    
    HIL_STM_JUMP,
    HIL_STM_LOCALTHROW,
    HIL_STM_THROW,
    HIL_STM_TRYCATCH,
    HIL_STM_RETURN
  } kind;
  union {
    struct {
      L lval;
      E exp;
    } assign;
    E exp;
    struct {
      E cond;      
      List_t then;    // List<S>      
      List_t elsee;   // List<S>
    } iff;
    struct {
      E cond;      
      List_t body;         // List<S>
      Label_t entryLabel;
      Label_t exitLabel;
      /* paddling the last part s3: 
       *   for (e1; e2; s3)
       *     s;
       */
      List_t padding;
    } doo;
    struct{
      List_t tryy;
      List_t catchh;
      // this is the label for "catch"
      Label_t label;
      Label_t end;
    }tryCatch;
    E returnn;
    Label_t jump;
    Label_t localThrow;
  } u;
};

S Hil_Stm_new_assign (L, E);
S Hil_Stm_new_exp (E);
S Hil_Stm_new_if (E, List_t, List_t);
S Hil_Stm_new_do (E cond
                  , List_t stms
                  , Label_t entryLabel
                  , Label_t exitLabel
                  , List_t padding);
S Hil_Stm_new_jump (Label_t jump);
S Hil_Stm_new_localThrow (Label_t label);
S Hil_Stm_new_throw ();
S Hil_Stm_new_tryCatch (List_t tryy
                        , List_t catchh
                        , Label_t label
                        , Label_t end);
S Hil_Stm_new_return (E);
File_t Hil_Stm_print (File_t file, S);

//////////////////////////////////////////////////////
/* function */
struct F
{
  Atype_t type;     // return type, from atoms/atype
  Id_t name;
  // List<Dec_t> , from atoms/dec
  List_t args;
  // List<Dec_t>, from atoms/dec
  List_t decs;
  // List<Stm_t> */
  List_t stms;
};

F Hil_Fun_new (Atype_t, Id_t, List_t, List_t, List_t);
File_t Hil_Fun_print (File_t file, F);

//////////////////////////////////////////////////////
// Program
struct P
{
  // List<Class_t>, where Class_t is imported from atoms/
  List_t classes;
  List_t funcs;
};

P Hil_Prog_new (List_t classes, List_t funcs);
File_t Hil_Prog_print (File_t file, P x);

#undef E
#undef F
#undef L
#undef P
#undef S

#endif
