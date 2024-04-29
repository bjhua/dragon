#include "x86.h"
#include "../lib/char-buffer.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include "../lib/todo.h"
#include <assert.h>


#define P X86_Prog_t
#define Str X86_Str_t
#define F X86_Fun_t
#define S X86_Stm_t
#define O X86_Operand_t
#define D X86_Dec_t
#define Stt X86_Struct_t
#define M X86_Mask_t
#define R X86_Register_t


#define ONEM (1024 * 1024)
#define NUM 16

static char buffer[16 * ONEM];

static int writeNums = 0;

static void buffer_init(void) {
    /* int n = (Control_bufferSize>0) */
    /*   ? (Control_bufferSize): NUM; */
    /* n *= ONEM; */
    /* Mem_NEW_SIZE (buffer, n); */
}

static void buffer_final(void) {
    /* buffer = 0; */
}

/* try to speed up this operation on rather large files */
static File_t file = 0;

static void file_init(File_t f) {
    file = f;
    setbuf(file, buffer);
}

static void print(String_t s) {
    writeNums++;
    fprintf(file, "%s", s);
    if (writeNums > 1000) {
        fflush(file);
        writeNums = 0;
    }
}

int X86_Register_equals(R r1, R r2) {
    return r1 == r2;
}

void X86_Register_print(R r) {
    switch (r) {
        case X86_AL:
            print("%al");
            return;
        case X86_EAX:
            print("%eax");
            return;
        case X86_EBX:
            print("%ebx");
            return;
        case X86_ECX:
            print("%ecx");
            return;
        case X86_EDX:
            print("%edx");
            return;
        case X86_EDI:
            print("%edi");
            return;
        case X86_ESI:
            print("%esi");
            return;
        case X86_EBP:
            print("%ebp");
            return;
        case X86_ESP:
            print("%esp");
            return;
        default:
            Error_impossible();
            return;
    }
    Error_impossible();
}

O X86_Operand_new_int(int i) {
    O e;
    Mem_NEW(e);
    e->kind = X86_OP_INT;
    e->u.intlit = i;
    return e;
}

O X86_Operand_new_global(Id_t id) {
    O e;
    Mem_NEW(e);
    e->kind = X86_OP_GLOBAL;
    e->u.global = id;
    return e;
}

O X86_Operand_new_inStack(int index) {
    O e;
    Mem_NEW(e);
    e->kind = X86_OP_INSTACK;
    e->u.index = index;
    return e;
}

O X86_Operand_new_reg(R r) {
    O e;
    Mem_NEW(e);
    e->kind = X86_OP_REG;
    e->u.reg = r;
    return e;
}

O X86_Operand_new_mem(R base, int offset) {
    O e;
    Mem_NEW(e);
    e->kind = X86_OP_MEM;
    e->u.mem.base = base;
    e->u.mem.offset = offset;
    return e;
}

int X86_Operand_sameStackSlot(O x, O y) {
    if (x->kind != X86_OP_INSTACK || y->kind != X86_OP_INSTACK)
        return 0;
    return x->u.index == y->u.index;
}

void X86_Operand_print(O o) {
    assert(o);
    switch (o->kind) {
        case X86_OP_INT:
            print("$");
            print(Int_toString(o->u.intlit));
            return;
        case X86_OP_GLOBAL:
            print("$");
            print(Id_toString(o->u.global));
            return;
        case X86_OP_INSTACK:
            print(Int_toString(o->u.index));
            print("(%ebp)");
            return;
        case X86_OP_REG:
            X86_Register_print(o->u.reg);
            return;
        case X86_OP_MEM:
            print(Int_toString(4 * (o->u.mem.offset)));
            print("(");
            X86_Register_print(o->u.mem.base);
            print(")");
            return;
        default:
            Error_impossible();
            return;
    }
    Error_impossible();
}

S X86_Stm_new_moverr(R dest, R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_MOVERR;
    s->u.moverr.dest = dest;
    s->u.moverr.src = src;
    return s;
}

S X86_Stm_new_moveri(R dest, int src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_MOVERI;
    s->u.moveri.dest = dest;
    s->u.moveri.src = src;
    return s;
}

S X86_Stm_new_load(R dest, O src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_LOAD;
    s->u.load.dest = dest;
    s->u.load.src = src;
    return s;
}

S X86_Stm_new_store(O dest, R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_STORE;
    s->u.store.dest = dest;
    s->u.store.src = src;
    return s;
}

S X86_Stm_new_bop(R dest, Operator_t opr,
                  R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_BOP;
    s->u.bop.src = src;
    s->u.bop.op = opr;
    s->u.bop.dest = dest;
    return s;
}

S X86_Stm_new_uop(R dest, Operator_t opr, R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_UOP;
    s->u.uop.dest = dest;
    s->u.uop.src = src;
    s->u.uop.op = opr;
    return s;
}

S X86_Stm_new_call(Id_t f) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_CALL;
    s->u.call.name = f;
    return s;
}

S X86_Stm_new_cmp(R dest, R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_CMP;
    s->u.cmp.src = src;
    s->u.cmp.dest = dest;
    return s;
}

S X86_Stm_new_label(Label_t label) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_LABEL;
    s->u.label = label;
    return s;
}

S X86_Stm_new_je(Label_t label) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_JE;
    s->u.je = label;
    return s;
}

S X86_Stm_new_jl(Label_t label) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_JL;
    s->u.jl = label;
    return s;
}

S X86_Stm_new_jump(Label_t label) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_JUMP;
    s->u.jump = label;
    return s;
}

S X86_Stm_new_push(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_PUSH;
    s->u.push = r;
    return s;
}

S X86_Stm_new_neg(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_NEG;
    s->u.neg = r;
    return s;
}

S X86_Stm_new_setl(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETL;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_setle(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETLE;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_setg(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETG;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_setge(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETGE;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_sete(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETE;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_setne(X86_Register_t r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_SETNE;
    s->u.setAny = r;
    return s;
}

S X86_Stm_new_extendAl(void) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_EXTENDAL;
    return s;
}

S X86_Stm_new_return(void) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_RETURN;
    return s;
}

S X86_Stm_new_cltd(void) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_CLTD;
    return s;
}

S X86_Stm_new_xor(R dest, R src) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_XOR;
    s->u.xor.dest = dest;
    s->u.xor.src = src;
    return s;
}

S X86_Stm_new_not(R r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_NOT;
    s->u.not = r;
    return s;
}

S X86_Stm_new_inc(R r) {
    S s;
    Mem_NEW(s);
    s->kind = X86_STM_INC;
    s->u.inc = r;
    return s;
}

static void X86_Operator_print(Operator_t o) {
    switch (o) {
        case OP_ADD:
            print("addl ");
            return;
        case OP_SUB:
            print("subl ");
            return;
        case OP_TIMES:
            print("imull ");
            return;
        case OP_DIVIDE:
            print("idivl ");
            return;
        default:
            TODO;
            Error_impossible();
    }
    Error_impossible();
}

static void space4(void) {
    print("\t");
}

void X86_Stm_print(S s) {
    assert(s);
    switch (s->kind) {
        case X86_STM_MOVERR:
            space4();
            print("movl ");
            X86_Register_print(s->u.moverr.src);
            print(", ");
            X86_Register_print(s->u.moverr.dest);
            break;
        case X86_STM_MOVERI:
            space4();
            print("movl $");
            print(Int_toString(s->u.moveri.src));
            print(", ");
            X86_Register_print(s->u.moveri.dest);
            break;
        case X86_STM_LOAD:
            space4();
            print("movl ");
            X86_Operand_print(s->u.load.src);
            print(", ");
            X86_Register_print(s->u.load.dest);
            break;
        case X86_STM_STORE:
            space4();
            print("movl ");
            X86_Register_print(s->u.store.src);
            print(", ");
            X86_Operand_print(s->u.store.dest);
            break;
        case X86_STM_BOP:
            space4();
            X86_Operator_print(s->u.bop.op);
            X86_Register_print(s->u.bop.src);
            print(", ");
            X86_Register_print(s->u.bop.dest);
            break;
        case X86_STM_UOP:
            space4();
            X86_Operator_print(s->u.uop.op);
            X86_Register_print(s->u.uop.dest);
            break;
        case X86_STM_CALL:
            space4();
            print("call ");
            print(Id_toString(s->u.call.name));
            break;
        case X86_STM_CMP:
            space4();
            print("cmpl ");
            X86_Register_print(s->u.cmp.src);
            print(", ");
            X86_Register_print(s->u.cmp.dest);
            break;
        case X86_STM_LABEL:
            print(Label_toString(s->u.label));
            print(":");
            break;
        case X86_STM_JE:
            space4();
            print("je ");
            print(Label_toString(s->u.je));
            break;
        case X86_STM_JL:
            space4();
            print("jl ");
            print(Label_toString(s->u.je));
            break;
        case X86_STM_JUMP:
            space4();
            print("jmp ");
            print(Label_toString(s->u.jump));
            break;
        case X86_STM_PUSH:
            space4();
            print("pushl ");
            X86_Register_print(s->u.push);
            break;
        case X86_STM_NEG:
            space4();
            print("negl ");
            X86_Register_print(s->u.neg);
            break;
        case X86_STM_SETL:
            space4();
            print("setl ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_SETLE:
            space4();
            print("setle ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_SETG:
            space4();
            print("setg ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_SETGE:
            space4();
            print("setge ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_SETE:
            space4();
            print("sete ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_SETNE:
            space4();
            print("setne ");
            X86_Register_print(s->u.setAny);
            break;
        case X86_STM_XOR:
            space4();
            print("xorl ");
            X86_Register_print(s->u.xor.src);
            print(", ");
            X86_Register_print(s->u.xor.dest);
            break;
        case X86_STM_EXTENDAL:
            space4();
            print("cbtw\n");
            space4();
            print("cwtl");
            break;
        case X86_STM_NOT: {
            space4();
            print("notl ");
            X86_Register_print(s->u.not );
            break;
        }
        case X86_STM_RETURN:
            space4();
            print("leave\n\tret");
            break;
        case X86_STM_CLTD:
            space4();
            print("cltd");
            break;
        case X86_STM_INC:
            space4();
            print("incl ");
            X86_Register_print(s->u.inc);
            break;
        default:
            Error_impossible();
            break;
    }
    print("\n");
}

F X86_Fun_new(Id_t type, Id_t name, List_t args,
              List_t decs, List_t stms,
              Id_t retId,
              Label_t entry, Label_t exitt) {
    F f;
    Mem_NEW(f);
    f->type = type;
    f->name = name;
    f->args = args;
    f->decs = decs;
    f->stms = stms;
    f->retId = retId;
    f->entry = entry;
    f->exitt = exitt;
    return f;
}


//static void X86_Dec_print(X86_Struct_t);

void X86_Fun_print(F f) {
    assert(f);
    print("\t.text\n");
    print("\t.globl _");
    print(Id_toString(f->name));
    print("\n_");
    print(Id_toString(f->name));
    print(":\n");
    print("\tpushl %ebp\n");
    print("\tmovl %esp, %ebp\n");
    {
        int i = List_size(f->decs);
        if (i) {
            print("\tsubl $");
            print(Int_toString(4 * i));
            print(", %esp\n");
        }
    }
    List_foreach(f->stms,
                 (Poly_tyVoid) X86_Stm_print);
    print("\n\n");
}


Stt X86_Struct_new(Id_t type, Id_t var) {
    Stt x;
    Mem_NEW(x);
    x->type = type;
    x->var = var;
    return x;
}

void X86_Struct_print(Stt x) {
    assert(x);
    print(Id_toString(x->type));
    print(" ");
    print(Id_toString(x->var));
    print(";\n");
}

//static void X86_Dec_print(Stt x) {
//    assert(x);
//    print("\t");
//    print(Id_toString(x->type));
//    print(" ");
//    print(Id_toString(x->var));
//    print(";\n");
//}


Str X86_Str_new(Id_t name, String_t value) {
    Str d;
    Mem_NEW(d);
    d->name = name;
    d->value = value;
    return d;
}

static String_t convert(String_t s) {
    CharBuffer_t buffer_ = CharBuffer_new();

    while (*s) {
        char c = *s;
        switch (c) {
            case '\n':
                CharBuffer_append(buffer_, '\\');
                CharBuffer_append(buffer_, 'n');
                break;
            case '\t':
                CharBuffer_append(buffer_, '\\');
                CharBuffer_append(buffer_, 't');
                break;
            case '\\':
                CharBuffer_append(buffer_, '\\');
                CharBuffer_append(buffer_, '\\');
                break;
            case '\"':
                CharBuffer_append(buffer_, '\\');
                CharBuffer_append(buffer_, '\"');
                break;
            default:
                CharBuffer_append(buffer_, c);
                break;
        }
        s++;
    }
    return CharBuffer_toStringBeforeClear(buffer_);
}

void X86_Str_print(Str s) {
    print(Id_toString(s->name));
    print(":\n\t.string \"");
    print(convert(s->value));
    print("\"\n");
}

static void printStrs(List_t strings) {
    if (List_isEmpty(strings))
        return;
    print("\t.data\n");
    List_foreach(strings,
                 (Poly_tyVoid) X86_Str_print);
}

M X86_Mask_new(Id_t name, int size, List_t index) {
    M m;
    Mem_NEW(m);
    m->name = name;
    m->size = size;
    m->index = index;
    return m;
}

File_t X86_Mask_print(File_t file_, M m) {
    List_t p;

    assert(m);
    fprintf(file_, "%s", Id_toString(m->name));
    fprintf(file_, ":\n\t.int ");
    fprintf(file_, "%s", Int_toString(m->size));
    if (List_isEmpty(m->index)) {
        fprintf(file_, "\n");
        return file_;
    }
    fprintf(file_, ", ");
    p = List_getFirst(m->index);
    while (p) {
        fprintf(file_, "%s", Int_toString((long) p->data));
        if (p->next)
            fprintf(file_, ", ");
        p = p->next;
    }
    fprintf(file_, "\n");
    return file_;
}

static void printMask(File_t file_, List_t ms) {
    if (List_isEmpty(ms))
        return;

    fprintf(file_, "\t.data\n"
                   "\t.align 8\n");
    List_foldl(ms, file_, (Poly_tyFold) X86_Mask_print);
}

P X86_Prog_new(List_t strings, List_t masks, List_t funcs) {
    P p;

    Mem_NEW(p);
    p->strings = strings;
    p->masks = masks;
    p->funcs = funcs;
    return p;
}

File_t X86_Prog_print(File_t file_, P p) {
    assert(file_);
    assert(p);

    file_init(file_);
    buffer_init();

    printStrs(p->strings);

    fprintf(file_, "%s", "\n");
    printMask(file_, p->masks);

    fprintf(file_, "%s", "\n");
    List_foldl(p->funcs, file_, (Poly_tyFold) X86_Fun_print);
    buffer_final();
    return file_;
}

#undef P
#undef Str
#undef F
#undef S
#undef O
#undef M
#undef D
#undef Stt
#undef List_t
