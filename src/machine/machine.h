#ifndef MACHINE_H
#define MACHINE_H

#include "../atoms/atoms.h"
#include "../lib/file.h"
#include "../lib/list.h"
#include "../lib/string.h"

#define B Machine_Block_t
#define F Machine_Fun_t
#define I Machine_FrameInfo_t
#define J Machine_ObjInfo_t
#define M Machine_Mem_t
#define O Machine_Operand_t
#define P Machine_Prog_t
#define R Machine_Str_t
#define S Machine_Stm_t
#define T Machine_Transfer_t

typedef struct B *B;
typedef struct F *F;
typedef struct I *I;
typedef struct J *J;
typedef struct M *M;
typedef struct O *O;
typedef struct P *P;
typedef struct R *R;
typedef struct S *S;
typedef struct T *T;

///////////////////////////////////////////////////////
// operands
struct O {
    enum {
        MACHINE_OP_INT,
        MACHINE_OP_GLOBAL,
        MACHINE_OP_ID,
    } kind;
    union {
        long int_lit;
        Id_t id;
    } u;
};

O Machine_Operand_new_int(long i);

O Machine_Operand_new_global(Id_t id);

O Machine_Operand_new_id(Id_t id);

File_t Machine_Operand_print(File_t file, O);

///////////////////////////////////////////////////////
// memory
struct M {
    enum {
        MACHINE_MEM_ARRAY,
        MACHINE_MEM_CLASS
    } kind;
    union {
        struct {
            Id_t name;
            O index;
        } array;
        struct {
            Id_t name;
            Id_t field;
            // starting from 0
            long index;
        } class;
    } u;
};

M Machine_Mem_new_array(Id_t, O);

M Machine_Mem_new_class(Id_t, Id_t, long);

File_t Machine_Mem_print(File_t file, M m);


///////////////////////////////////////////////////////
// statements
struct S {
    enum {
        MACHINE_STM_MOVE,
        MACHINE_STM_BOP,
        MACHINE_STM_UOP,
        MACHINE_STM_CALL,
        MACHINE_STM_CALL_NOASSIGN,
        MACHINE_STM_STORE,
        MACHINE_STM_LOAD,
        MACHINE_STM_TRY,
        MACHINE_STM_TRY_END,
        MACHINE_STM_NEW_CLASS,
        MACHINE_STM_NEW_ARRAY,
        MACHINE_STM_RUNTIME_CLASS,
        MACHINE_STM_RUNTIME_ARRAY
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

        struct {
            Id_t name;
            /* List<O> */
            List_t args;
            Label_t normal;
        } callnoassign;
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
            // object layout index, effective when index >= 1.
            long index;
            // length of this object
            long size;
            // runtime name of this function
            Id_t fname;
        } class;
        struct {
            Id_t dest;
            int isPtr;
            O size;
            // the above "size" is the number of the elements
            // thus we need this scale.
            int scale;
            // runtime name of this function
            Id_t fname;
        } array;
        Label_t try;
        Label_t tryEnd;
    } u;
};

S Machine_Stm_new_move(Id_t dest, O src);

S Machine_Stm_new_bop(Id_t dest, O left,
                      Operator_t op, O right);

S Machine_Stm_new_uop(Id_t dest, Operator_t opr, O src);

S Machine_Stm_new_store(M, O);

S Machine_Stm_new_load(Id_t, M);

S Machine_Stm_new_try(Label_t label);

S Machine_Stm_new_try_end(Label_t end);

S Machine_Stm_new_newClass(Id_t dest, Id_t className);

S Machine_Stm_new_newArray(Id_t, Atype_t, O size);

S Machine_Stm_Runtime_class(Id_t dest, long index, long size, Id_t fname);

S Machine_Stm_Runtime_array(Id_t, int isPtr, O size, int scale, Id_t fname);

File_t Machine_Stm_print(File_t file, S);


///////////////////////////////////////////////////////
/* transfer */
struct T {
    enum {
        MACHINE_TRANS_IF,
        MACHINE_TRANS_JUMP,
        MACHINE_TRANS_RETURN,
        MACHINE_TRANS_CALL,
        MACHINE_TRANS_CALL_NOASSIGN,
        MACHINE_TRANS_THROW
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
            Label_t leave;
            Label_t normal;
        } call;
        Label_t jump;
        O ret;
    } u;
};

T Machine_Transfer_new_if(O, Label_t, Label_t);

T Machine_Transfer_new_jump(Label_t);

T Machine_Transfer_new_return(O);

T Machine_Transfer_new_throw(void);

T Machine_Transfer_new_call(Id_t dest, Id_t name, List_t args, Label_t leave, Label_t normal);

T Machine_Transfer_new_callnoassign(Id_t name, List_t args, Label_t leave, Label_t normal);

File_t Machine_Transfer_print(File_t file, T);


///////////////////////////////////////////////////////
/* basic block */
struct B {
    Label_t label;
    // List<S>
    List_t stms;
    T transfer;
};

B Machine_Block_new(Label_t, List_t, T);

File_t Machine_Block_print(File_t f, B);


/////////////////////////////////////////////////////
/* function */
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
    int frameIndex;// index into "frameInfo"
};

F Machine_Fun_new(Atype_t type, Id_t name, List_t args, List_t decs, List_t blocks, Id_t retId, Label_t entry,
                  Label_t exitt, int frameIndex);

File_t Machine_Fun_print(File_t, F);

/////////////////////////////////////////////////////
// string (global vars)
struct R {
    Id_t name;
    String_t value;
};

R Machine_Str_new(Id_t, String_t);

File_t Machine_Str_print(File_t, R);


/////////////////////////////////////////////////////
// object info
struct J {
    /* List<int> */
    List_t offsets;
};

J Machine_ObjInfo_new(List_t);

File_t Machine_ObjInfo_print(File_t, J);


/////////////////////////////////////////////////////
// frame info
struct I {
    // this is a hack for now, and should be moved into
    // another structure.
    // This is the offsets for arguments.
    // List<int>
    List_t frameOffsets;
    // this is a hack for now, and should be moved into
    // another structure.
    // this is the offsets for declarations.
    // List<int>
    List_t frameOffsetsDec;
    // size of the entire frame, including args and locals,
    // but excluding return address and ... (in unit of
    // bytes)
    int size;
};

I Machine_FrameInfo_new(List_t, List_t, int);

File_t Machine_FrameInfo_print(File_t file, I);


/////////////////////////////////////////////////////
// program
struct P {
    // List<Mahine_Str_t>
    List_t strings;// a list of all strings
    // List<I>
    List_t frameInfo;// frame information for each func
    // List<J>
    List_t layoutInfo;
    // List<F>
    List_t classes;
    List_t funcs;
};

P Machine_Prog_new(List_t strings, List_t frameInfo, List_t layoutInfo, List_t classes, List_t funcs);

File_t Machine_Prog_print(File_t f, P x);

#undef B
#undef F
#undef I
#undef J
#undef M
#undef O
#undef P
#undef R
#undef S
#undef T

#endif
