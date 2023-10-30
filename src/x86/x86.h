#ifndef X86_H
#define X86_H

#include "../lib/list.h"
#include "../lib/string.h"
#include "../lib/file.h"
#include "../atoms/atoms.h"

#define P X86_Prog_t
#define Str X86_Str_t
#define F X86_Fun_t
#define S X86_Stm_t
#define O X86_Operand_t
#define D X86_Dec_t
#define Stt X86_Struct_t
#define M X86_Mask_t
#define R X86_Register_t

typedef struct O *O;
typedef struct S *S;
typedef struct Stt *Stt;
typedef struct M *M;
typedef struct F *F;
typedef struct Str *Str;
typedef struct D *D;
typedef struct P *P;

typedef enum {
    /* only to deal with set* instructions */
    X86_AL,
    X86_EAX,
    X86_EBX,
    X86_ECX,
    X86_EDX,
    X86_EDI,
    X86_ESI,
    X86_EBP,
    X86_ESP
} R;

int X86_Register_equals(R, R);

void X86_Register_print(R);

struct O {
    enum {
        X86_OP_INT,
        X86_OP_GLOBAL,
        X86_OP_INSTACK,
        X86_OP_REG,
        X86_OP_MEM
    } kind;
    union {
        int intlit;
        Id_t global;
        int index;
        R reg;
        struct {
            R base;
            int offset;
        } mem;
    } u;
};


O X86_Operand_new_int(int i);

O X86_Operand_new_global(Id_t id);

O X86_Operand_new_inStack(int index);

O X86_Operand_new_reg(R r);

O X86_Operand_new_mem(R r, int offset);

int X86_Operand_sameStackSlot(O x, O y);

void X86_Operand_print(O);

typedef enum {
    X86_OPR_ADD,
    X86_OPR_SUB,
    X86_OPR_TIMES,
    X86_OPR_DIVIDE,
    X86_OPR_MODUS
} Opr;

struct S {
    enum {
        X86_STM_MOVERR,
        X86_STM_MOVERI,
        X86_STM_LOAD,
        X86_STM_STORE,
        X86_STM_BOP,
        X86_STM_UOP,
        X86_STM_CALL,
        X86_STM_CMP,
        X86_STM_LABEL,
        X86_STM_JE,
        X86_STM_JL,
        X86_STM_JUMP,
        X86_STM_PUSH,
        X86_STM_RETURN,
        X86_STM_CLTD,
        X86_STM_NEG,
        X86_STM_SETL,
        X86_STM_SETLE,
        X86_STM_SETG,
        X86_STM_SETGE,
        X86_STM_SETE,
        X86_STM_SETNE,
        X86_STM_XOR,
        X86_STM_EXTENDAL,
        X86_STM_NOT,
        X86_STM_INC
    } kind;
    union {
        struct {
            R src;
            R dest;
        } moverr;
        struct {
            int src;
            R dest;
        } moveri;
        struct {
            O src;
            R dest;
        } load;
        struct {
            R src;
            O dest;
        } store;
        struct {
            R dest;
            Operator_t op;
            R src;
        } bop;
        struct {
            R src;
            Operator_t op;
            R dest;
        } uop;
        struct {
            Id_t name;
        } call;
        struct {
            R dest;
            R src;
        } cmp;
        R neg;
        R setAny;
        Label_t label;
        Label_t je;
        Label_t jl;
        Label_t jump;
        R push;
        struct {
            R dest;
            R src;
        } xor;
        R not;
        R inc;
    } u;
};

S X86_Stm_new_moverr(R dest, R src);

S X86_Stm_new_moveri(R dest, int i);

S X86_Stm_new_load(R dest, O src);

S X86_Stm_new_store(O dest, R src);

S X86_Stm_new_bop(R dest,
                  Operator_t op,
                  R src);

S X86_Stm_new_uop(R dest,
                  Operator_t opr,
                  R src);

S X86_Stm_new_call(Id_t name);

S X86_Stm_new_cmp(R dest,
                  R src);

S X86_Stm_new_push(R r);

S X86_Stm_new_label(Label_t label);

S X86_Stm_new_je(Label_t label);

S X86_Stm_new_jl(Label_t label);

S X86_Stm_new_jump(Label_t label);

S X86_Stm_new_return();

S X86_Stm_new_cltd();

S X86_Stm_new_neg(R r);

S X86_Stm_new_setl(R r);

S X86_Stm_new_setle(R r);

S X86_Stm_new_setg(R r);

S X86_Stm_new_setge(R r);

S X86_Stm_new_sete(R r);

S X86_Stm_new_setne(R r);

S X86_Stm_new_xor(R dest, R src);

S X86_Stm_new_extendAl();

S X86_Stm_new_not(R r);

S X86_Stm_new_inc(R r);

void X86_Stm_print(S);

/* function */
struct F {
    Id_t type;
    Id_t name;
    /* List<Stt> */
    List_t args;
    /* List<Struct_t> */
    List_t decs;
    /* List<Stm_t> */
    List_t stms;
    Id_t retId;
    Label_t entry;
    Label_t exitt;
};

F X86_Fun_new(Id_t, Id_t, List_t, List_t, List_t,
              Id_t, Label_t, Label_t);

void X86_Fun_print(F);

/* struct fields & function parameters */
/* this should be rewritten into two different types */
struct Stt {
    Id_t type;
    Id_t var;
};

Stt X86_Struct_new(Id_t, Id_t);

void X86_Struct_print(Stt);

struct Str {
    Id_t name;
    String_t value;
};

Str X86_Str_new(Id_t, String_t);

void X86_Str_print(Str);

struct M {
    Id_t name;
    int size;
    /* List<int> */
    List_t index;
};

M X86_Mask_new(Id_t name, int size, List_t);

File_t X86_Mask_print(File_t file, M);

struct P {
    /* List<Str> */
    List_t strings;
    /* List<M> */
    List_t masks;
    List_t funcs;
};

P X86_Prog_new(List_t strings,
               List_t masks,
               List_t funcs);

File_t X86_Prog_print(File_t file, P p);

#undef P
#undef Str
#undef F
#undef S
#undef O
#undef M
#undef D
#undef Stt
#undef R

#endif
