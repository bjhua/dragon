#ifndef SSA_H
#define SSA_H

#include "../atoms/atoms.h"
#include "../lib/file.h"
#include "../lib/graph.h"
#include "../lib/list.h"
#include "../lib/set.h"

#define B Ssa_Block_t
#define E Ssa_Exp_t
#define F Ssa_Fun_t
#define M Ssa_Mem_t
#define O Ssa_Operand_t
#define P Ssa_Prog_t
#define S Ssa_Stm_t
#define T Ssa_Transfer_t

typedef struct B *B;
typedef struct F *F;
typedef struct M *M;
typedef struct O *O;
typedef struct P *P;
typedef struct S *S;
typedef struct T *T;

///////////////////////////////////////////////////////
// operands
struct O {
    enum {
        SSA_OP_INT,
        SSA_OP_STR,
        SSA_OP_ID,
    } kind;
    union {
        long intlit;
        String_t strlit;
        Id_t id;
    } u;
};

O Ssa_Operand_new_int(long i);
O Ssa_Operand_new_string(String_t strlit);
O Ssa_Operand_new_id(Id_t id);
long Ssa_Operand_equals(O, O);
long Ssa_Operand_isConst(O o);
// given a list of operands, are they all same constants?
//long Ssa_Operand_isSameConst(List_t os);
File_t Ssa_Operand_print(File_t file, O);
String_t Ssa_Operand_toString(O);

///////////////////////////////////////////////////////
// memory
struct M {
    enum {
        SSA_MEM_ARRAY,
        SSA_MEM_CLASS
    } kind;
    union {
        struct {
            Id_t name;
            O index;
        } array;
        struct {
            Id_t name;
            Id_t field;
        } class;
    } u;
};

M Ssa_Mem_new_array(Id_t, O);
M Ssa_Mem_new_class(Id_t, Id_t);
File_t Ssa_Mem_print(File_t file, M m);

///////////////////////////////////////////////////////
// expression
struct E {
    enum {
        SSA_EXP_CONST,
        SSA_EXP_ID
    } kind;
    union {
        struct {
            Id_t name;
            O index;
        } const_;
        struct {
            Id_t id;
        } Id;
    } u;
};

// end of expression

///////////////////////////////////////////////////////
// statements
// phi argument
typedef struct {
    O arg;
    B pred;
} *Ssa_Stm_PhiArg_t;

Ssa_Stm_PhiArg_t Ssa_Stm_PhiArg_new(O, B pred);
int Ssa_Stm_PhiArg_equals(Ssa_Stm_PhiArg_t, Ssa_Stm_PhiArg_t);
//int Ssa_Stm_PhiArg_isSameCosnt(List_t args);
File_t Ssa_Stm_PhiArg_print(File_t, Ssa_Stm_PhiArg_t);
String_t Ssa_Stm_PhiArg_toString(Ssa_Stm_PhiArg_t);

struct S {
    enum {
        SSA_STM_MOVE,
        SSA_STM_BOP,
        SSA_STM_UOP,
        SSA_STM_CALL,
        SSA_STM_STORE,
        SSA_STM_LOAD,
        SSA_STM_NEW_CLASS,
        SSA_STM_NEW_ARRAY,
        SSA_STM_TRY,
        // The semantics for "end" is that the top
        // exception frame can be simply poped.
        SSA_STM_TRY_END,
        SSA_STM_PHI
    } kind;
    union {
        struct {
            Id_t dest;
            O src;
        } move;
        struct {
            Id_t dest;
            O left;
            Operator_t op;
            O right;
        } bop;
        struct {
            Id_t dest;
            Operator_t op;
            O src;
        } uop;

        // id[index] <- src
        struct {
            M m;
            O src;
        } store;
        struct {
            Id_t dest;
            M m;
        } load;
        struct {
            Id_t dest;
            Id_t cname;
        } newClass;
        struct {
            Id_t dest;
            Atype_t ty;
            O size;
        } newArray;
        struct {
            Id_t dest;
            // List<PhiArg_t>
            List_t args;
        } phi;
        Label_t try;
        Label_t tryEnd;
    } u;
};

S Ssa_Stm_new_move(Id_t dest, O src);
S Ssa_Stm_new_bop(Id_t dest,
                  O left,
                  Operator_t op,
                  O right);
S Ssa_Stm_new_uop(Id_t dest, Operator_t opr, O src);
//S Ssa_Stm_new_call(Id_t dest, Id_t name, List_t args, int);
S Ssa_Stm_new_store(M, O);
S Ssa_Stm_new_load(Id_t, M);
S Ssa_Stm_new_newClass(Id_t, Id_t);
S Ssa_Stm_new_newArray(Id_t, Atype_t, O size);
S Ssa_Stm_new_try(Label_t label);
S Ssa_Stm_new_try_end(Label_t label);
// preds: List<Block_t>, a list of predessors
//S Ssa_Stm_new_phi_preds(Id_t, List_t preds);
// for every use of "id" in s, apply "f" to "id", if the
// result "src" is nonzero, rewrite the "id" to "src".
S Ssa_Stm_renameUse2Op(S, O (*f)(Id_t));
int Ssa_Stm_PhiArg_isSameConst(List_t);
S Ssa_Stm_new_phi_predsBlock(Id_t var, List_t preds);
// including phi
void Ssa_Stm_foreachUse(S, void (*f)(Id_t));
File_t Ssa_Stm_print(File_t file, S);

///////////////////////////////////////////////////////
/* transfer */
struct T {
    enum {
        SSA_TRANS_IF,
        SSA_TRANS_JUMP,
        SSA_TRANS_RETURN,
        SSA_TRANS_CALL,
        SSA_TRANS_THROW
    } kind;
    union {
        struct {
            O cond;
            Label_t truee;
            Label_t falsee;
        } iff;
        struct {
            // assign to dest only for nonzero values
            // for now, this only happens after "deadcode" pass.
            // should be rewritten into another case.
            Id_t dest;
            Id_t name;
            /* List<O> */
            List_t args;
            // if leave==0, then this call may leave the
            // current function.
            // otherwise (for nonzero values), this call may
            // jump to local handler.
            Label_t leave;
            // normal indicates that
            Label_t normal;
        } call;
        Label_t jump;
        O ret;
    } u;
};

T Ssa_Transfer_new_if(O, Label_t, Label_t);
T Ssa_Transfer_new_jump(Label_t);
T Ssa_Transfer_new_return(O);
T Ssa_Transfer_new_throw(void);
T Ssa_Transfer_new_call(Id_t dest, Id_t f, List_t args, Label_t leave, Label_t normal);
// whether this transfer can jump to label "l"
int Ssa_Transfer_canJumpTo(T, Label_t l);
// rename labels only for non-zero label arguments.
T Ssa_Transfer_renameLabels_if(T, Label_t t, Label_t f);
// rename labels only for non-zero label arguments.
T Ssa_Transfer_renameLabels_jump(T, Label_t l);
// rename labels only for non-zero label arguments.
//T Ssa_Transfer_renameLables_call(T, Label_t);
T Ssa_Transfer_renameUse2Op(T, O (*f)(Id_t));
Set_t Ssa_Transfer_getDefIds(T);
void Ssa_Transfer_foreachDef(T, void (*f)(Id_t));
File_t Ssa_Transfer_print(File_t file, T);

///////////////////////////////////////////////////////
/* basic block */
struct B {
    Label_t label;
    // List_t phis;
    // List<S>
    List_t stms;
    T transfer;
};

B Ssa_Block_new(Label_t, List_t, T);

// whether this block can jump to label "l"
//int Ssa_Block_canJumpTo(B, Label_t l);
// apply "use" to each use in this block (including
// transfer but excluding phi). apply def to each
// definition, including phi.
B Ssa_Block_renameUseDefNoPhiUse(B, Id_t (*use)(Id_t), Id_t (*def)(Id_t));
void Ssa_Block_renamePhiArgPre(B current, B predesor, void (*f)(B, B, Id_t));
// subst: List<Tuple<Ssa_Stm_PhiArg_t, Id_t>>
B Ssa_Block_renamePhiArgs(B b, List_t subst);
// including phi
void Ssa_Block_foreachDef(B b, void (*f)(Id_t));
// including phi arguments
void Ssa_Block_foreachUse(B b, void (*f)(Id_t));
// get all identifiers which are defined in the block "b"
Set_t Ssa_Block_getDefIds(B b);
// get the property list from label in this block
Plist_t Ssa_Block_plist(B b);
int Ssa_Block_equals(B b1, B b2);
//int Ssa_Block_numSuccs(B b);
File_t Ssa_Block_print(File_t f, B);
File_t Ssa_Block_printForDot(File_t f, B);

///////////////////////////////////////////////////////
// function
struct F {
    Atype_t type;
    Id_t name;
    /* List<Dec_t> */
    List_t args;
    /* List<Dec_t> */
    List_t decs;
    /* List<B> */
    List_t blocks;
    Id_t retId;
    Label_t entry;
    Label_t exitt;
};

F Ssa_Fun_new(Atype_t,
              Id_t,
              List_t,
              List_t,
              List_t,
              Id_t retId,
              Label_t,
              Label_t);
File_t Ssa_Fun_print(File_t file, F);
B Ssa_Fun_searchLabel(F, Label_t l);
void Ssa_Fun_toDot(F, String_t figname);
// convert a function to a directed graph
Graph_t Ssa_Fun_toGraph(F f);
// generating SSA-form
//F Ssa_Fun_toSsa(F f);

//////////////////////////////////////////////////////
// programs
struct P {
    List_t classes;
    List_t funcs;
};

P Ssa_Prog_new(List_t classes, List_t funcs);
File_t Ssa_Prog_print(File_t f, P x);
void Ssa_Prog_toDot(P x, String_t file_name);

#undef B
#undef F
#undef M
#undef O
#undef P
#undef S
#undef T

#endif
