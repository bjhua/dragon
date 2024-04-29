#include "machine.h"
#include "../lib/mem.h"
#include <assert.h>

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

///////////////////////////////////////////////////////
/* operands */
O Machine_Operand_new_int(long i) {
    O e;

    Mem_NEW(e);
    e->kind = MACHINE_OP_INT;
    e->u.intlit = i;
    return e;
}

O Machine_Operand_new_global(Id_t id) {
    O e;

    Mem_NEW(e);
    e->kind = MACHINE_OP_GLOBAL;
    e->u.id = id;
    return e;
}


O Machine_Operand_new_id(Id_t id) {
    O e;

    Mem_NEW(e);
    e->kind = MACHINE_OP_ID;
    e->u.id = id;
    return e;
}

File_t Machine_Operand_print(File_t file, O o) {
    assert(o);
    switch (o->kind) {
        case MACHINE_OP_INT:
            fprintf(file, "%ld", o->u.intlit);
            break;
        case MACHINE_OP_GLOBAL:
            fprintf(file, "G_%s", Id_toString(o->u.id));
            break;
        case MACHINE_OP_ID:
            fprintf(file, "%s", Id_toString(o->u.id));
            break;
        default:
            Error_impossible();
            break;
    }
    return file;
}

///////////////////////////////////////////////////////
// memory
M Machine_Mem_new_array(Id_t name, O index) {
    M m;

    Mem_NEW(m);
    m->kind = MACHINE_MEM_ARRAY;
    m->u.array.name = name;
    m->u.array.index = index;
    return m;
}

M Machine_Mem_new_class(Id_t name, Id_t field, long index) {
    M m;

    Mem_NEW(m);
    m->kind = MACHINE_MEM_CLASS;
    m->u.class.name = name;
    m->u.class.field = field;
    m->u.class.index = index;
    return m;
}

File_t Machine_Mem_print(File_t file, M m) {
    assert(m);
    switch (m->kind) {
        case MACHINE_MEM_ARRAY:
            fprintf(file, "%s", Id_toString(m->u.array.name));
            fprintf(file, "%s", "[");
            Machine_Operand_print(file, m->u.array.index);
            fprintf(file, "%s", "]");
            break;
        case MACHINE_MEM_CLASS:
            fprintf(file, "%s", Id_toString(m->u.class.name));
            fprintf(file, "%s", ".");
            fprintf(file, "%s", Id_toString(m->u.class.field));
            fprintf(file, "%s%ld", "(INDEX=", m->u.class.index);
            fprintf(file, "%s", ")");
            break;
        default:
            Error_impossible();
            break;
    }
    return file;
}

//////////////////////////////////////////////////////
// statement
S Machine_Stm_new_move(Id_t dest, O src) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_MOVE;
    s->u.move.dest = dest;
    s->u.move.src = src;
    return s;
}

S Machine_Stm_new_bop(Id_t dest, O left, Operator_t opr, O right) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_BOP;
    s->u.bop.dest = dest;
    s->u.bop.left = left;
    s->u.bop.op = opr;
    s->u.bop.right = right;
    return s;
}

S Machine_Stm_new_uop(Id_t dest, Operator_t opr, O src) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_UOP;
    s->u.uop.dest = dest;
    s->u.uop.op = opr;
    s->u.uop.src = src;
    return s;
}


S Machine_Stm_new_store(M m, O src) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_STORE;
    s->u.store.m = m;
    s->u.store.src = src;
    return s;
}

S Machine_Stm_new_load(Id_t dest, M m) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_LOAD;
    s->u.load.dest = dest;
    s->u.load.m = m;
    return s;
}

S Machine_Stm_new_try(Label_t label) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_TRY;
    s->u.try = label;
    return s;
}

S Machine_Stm_new_try_end(Label_t tryEnd) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_TRY_END;
    s->u.tryEnd = tryEnd;
    return s;
}

S Machine_Stm_new_newClass(Id_t dest, Id_t cname) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_NEW_CLASS;
    s->u.newClass.dest = dest;
    s->u.newClass.cname = cname;
    return s;
}

S Machine_Stm_new_newArray(Id_t dest, Atype_t type, O size) {
    S s;
    Mem_NEW(s);
    s->kind = MACHINE_STM_NEW_ARRAY;
    s->u.newArray.dest = dest;
    s->u.newArray.ty = type;
    s->u.newArray.size = size;
    return s;
}

S Machine_Stm_Runtime_class(Id_t dest, long index, long size, Id_t fname) {
    S s;

    Mem_NEW(s);
    s->kind = MACHINE_STM_RUNTIME_CLASS;
    s->u.class.dest = dest;
    s->u.class.index = index;
    s->u.class.size = size;
    s->u.class.fname = fname;
    return s;
}

S Machine_Stm_Runtime_array(Id_t dest, int isPtr, O size, int scale, Id_t file_name) {
    S s;
    Mem_NEW(s);
    s->kind = MACHINE_STM_RUNTIME_ARRAY;
    s->u.array.dest = dest;
    s->u.array.isPtr = isPtr;
    s->u.array.size = size;
    s->u.array.scale = scale;
    s->u.array.fname = file_name;
    return s;
}

static void spacetab(File_t file) {
    fprintf(file, "%s", "\t");
}

File_t Machine_Stm_print(File_t file, S s) {
    assert(s);
    spacetab(file);
    switch (s->kind) {
        case MACHINE_STM_MOVE:
            fprintf(file, "%s", Id_toString(s->u.move.dest));
            fprintf(file, "%s", " = ");
            Machine_Operand_print(file, s->u.move.src);
            break;
        case MACHINE_STM_BOP:
            fprintf(file, "%s", Id_toString(s->u.bop.dest));
            fprintf(file, "%s", " = ");
            Machine_Operand_print(file, s->u.bop.left);
            fprintf(file, "%s", Operator_toString(s->u.bop.op));
            Machine_Operand_print(file, s->u.bop.right);
            break;
        case MACHINE_STM_UOP:
            fprintf(file, "%s", Id_toString(s->u.uop.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Operator_toString(s->u.uop.op));
            Machine_Operand_print(file, s->u.uop.src);
            break;

        case MACHINE_STM_STORE:
            Machine_Mem_print(file, s->u.store.m);
            fprintf(file, "%s", " = ");
            Machine_Operand_print(file, s->u.store.src);
            break;
        case MACHINE_STM_LOAD:
            fprintf(file, "%s", Id_toString(s->u.load.dest));
            fprintf(file, "%s", " = ");
            Machine_Mem_print(file, s->u.load.m);
            break;
        case MACHINE_STM_TRY: {
            fprintf(file, "try (%s)", Label_toString(s->u.try));
            break;
        }
        case MACHINE_STM_TRY_END: {
            fprintf(file, "try_end(%s)", Label_toString(s->u.tryEnd));
            break;
        }
        case MACHINE_STM_NEW_CLASS: {
            fprintf(file, "%s", Id_toString(s->u.newClass.dest));
            fprintf(file, "%s", " = new ");
            fprintf(file, "%s", Id_toString(s->u.newClass.cname));
            fprintf(file, "%s", " ()");
            break;
        }
        case MACHINE_STM_NEW_ARRAY: {
            fprintf(file, "%s", Id_toString(s->u.newArray.dest));
            fprintf(file, "%s", " = new ");
            fprintf(file, "%s", Atype_toString(s->u.newArray.ty));
            fprintf(file, "%s", "[");
            Machine_Operand_print(file, s->u.newArray.size);
            fprintf(file, "%s", "]");
            break;
        }
        case MACHINE_STM_RUNTIME_CLASS: {
            fprintf(file, "%s", Id_toString(s->u.class.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(s->u.class.fname));
            fprintf(file, "%s", " (index = ");
            fprintf(file, "%ld, ", s->u.class.index);
            fprintf(file, "%s", "size = ");
            fprintf(file, "%ld", s->u.class.size);
            fprintf(file, "%s", ")");
            break;
        }
        case MACHINE_STM_RUNTIME_ARRAY: {
            fprintf(file, "%s", Id_toString(s->u.array.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(s->u.array.fname));
            fprintf(file, "%s", "(isPtr = ");
            fprintf(file, "%d", s->u.array.isPtr);
            fprintf(file, "%s", ", size = ");
            Machine_Operand_print(file, s->u.array.size);
            fprintf(file, "%s", ", scale = ");
            fprintf(file, "%d", s->u.array.scale);
            fprintf(file, "%s", ")");
            break;
        }
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible();
            break;
    }
    fprintf(file, "%s", ";\n");
    return file;
}


///////////////////////////////////////////////////////
/* transfer */
T Machine_Transfer_new_if(O cond, Label_t tt, Label_t f) {
    T t;

    Mem_NEW(t);
    t->kind = MACHINE_TRANS_IF;
    t->u.iff.cond = cond;
    t->u.iff.truee = tt;
    t->u.iff.falsee = f;
    return t;
}

T Machine_Transfer_new_jump(Label_t l) {
    T t;

    Mem_NEW(t);
    t->kind = MACHINE_TRANS_JUMP;
    t->u.jump = l;
    return t;
}

T Machine_Transfer_new_return(O r) {
    T t;

    Mem_NEW(t);
    t->kind = MACHINE_TRANS_RETURN;
    t->u.ret = r;
    return t;
}

T Machine_Transfer_new_throw(void) {
    T t;

    Mem_NEW(t);
    t->kind = MACHINE_TRANS_THROW;
    return t;
}

T Machine_Transfer_new_call(Id_t dest, Id_t f, List_t args, Label_t leave, Label_t normal) {
    T s;

    Mem_NEW(s);
    s->kind = MACHINE_TRANS_CALL;
    s->u.call.dest = dest;
    s->u.call.name = f;
    s->u.call.args = args;
    s->u.call.leave = leave;
    s->u.call.normal = normal;
    return s;
}

T Machine_Transfer_new_callnoassign(Id_t f, List_t args, Label_t leave, Label_t normal) {
    T s;

    Mem_NEW(s);
    s->kind = MACHINE_TRANS_CALL_NOASSIGN;
    s->u.call.name = f;
    s->u.call.args = args;
    s->u.call.leave = leave;
    s->u.call.normal = normal;
    return s;
}


File_t Machine_Transfer_print(File_t file, T t) {
    assert(t);
    spacetab(file);
    switch (t->kind) {
        case MACHINE_TRANS_IF:
            fprintf(file, "%s", "if (");
            Machine_Operand_print(file, t->u.iff.cond);
            fprintf(file, "%s", ", T=");
            fprintf(file, "%s", Label_toString(t->u.iff.truee));
            fprintf(file, "%s", ", F=");
            fprintf(file, "%s", Label_toString(t->u.iff.falsee));
            fprintf(file, "%s", ")");
            break;
        case MACHINE_TRANS_JUMP:
            fprintf(file, "%s", "jmp ");
            fprintf(file, "%s", Label_toString(t->u.jump));
            break;
        case MACHINE_TRANS_RETURN:
            fprintf(file, "%s", "return ");
            Machine_Operand_print(file, t->u.ret);
            break;
        case MACHINE_TRANS_THROW:
            fprintf(file, "%s", "throw");
            break;
        case MACHINE_TRANS_CALL:
            fprintf(file, "%s", Id_toString(t->u.call.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(t->u.call.name));
            fprintf(file, "%s", "(");
            List_print(t->u.call.args, ", ", file, (Poly_tyListPrint) Machine_Operand_print);
            fprintf(file, "%s", ");");
            fprintf(file, "goto %s", Label_toString(t->u.call.normal));
            break;
        case MACHINE_TRANS_CALL_NOASSIGN:
            fprintf(file, "%s", Id_toString(t->u.call.name));
            fprintf(file, "%s", "(");
            List_print(t->u.call.args, ", ", file, (Poly_tyListPrint) Machine_Operand_print);
            fprintf(file, "%s", ")");
            fprintf(file, "goto %s", Label_toString(t->u.call.normal));
            break;
        default:
            Error_impossible();
            break;
    }
    fprintf(file, "%s", "\n");
    return file;
}

///////////////////////////////////////////////////////
/* basic block */
B Machine_Block_new(Label_t label, List_t stms, T t) {
    B b;

    Mem_NEW(b);
    b->label = label;
    b->stms = stms;
    b->transfer = t;
    return b;
}

File_t Machine_Block_print(File_t file, B b) {
    assert(b);

    fprintf(file, "%s", Label_toString(b->label));
    fprintf(file, "%s", ":\n");
    List_foldl(b->stms, file, (Poly_tyFold) Machine_Stm_print);
    Machine_Transfer_print(file, b->transfer);
    fprintf(file, "%s", "\n");
    return file;
}


/////////////////////////////////////////////////////
/* function */
F Machine_Fun_new(Atype_t type, Id_t name, List_t args, List_t decs, List_t blocks, Id_t retId, Label_t entry,
                  Label_t exitt, int frameIndex) {
    F f;

    Mem_NEW(f);
    f->type = type;
    f->name = name;
    f->args = args;
    f->decs = decs;
    f->blocks = blocks;
    f->retId = retId;
    f->entry = entry;
    f->exitt = exitt;
    f->frameIndex = frameIndex;
    return f;
}

File_t Machine_Fun_print(File_t file, F f) {
    assert(f);

    fprintf(file, "%s ", Atype_toString(f->type));
    fprintf(file, "%s", Id_toString(f->name));
    fprintf(file, "%s", "(");
    List_foldl(f->args, file, (Poly_tyFold) Dec_printAsArg);
    fprintf(file, "%s", ")\n{\n");
    List_foldl(f->decs, file, (Poly_tyFold) Dec_printAsLocal);
    fprintf(file, "\n");
    fprintf(file, "%s", "Fentry = ");
    fprintf(file, "%s", Label_toString(f->entry));
    fprintf(file, "%s", ", Fexit = ");
    fprintf(file, "%s", Label_toString(f->exitt));
    fprintf(file, "%s", "\n");
    List_foldl(f->blocks, file, (Poly_tyFold) Machine_Block_print);
    fprintf(file, "%s = %d", "FRAME_INDEX ", f->frameIndex);
    fprintf(file, "%s", "}\n\n");
    return file;
}


/////////////////////////////////////////////////////
// string (global vars)
R Machine_Str_new(Id_t name, String_t value) {
    R d;

    Mem_NEW(d);
    d->name = name;
    d->value = value;
    return d;
}

File_t Machine_Str_print(File_t file, R s) {
    fprintf(file, "%s", "string ");
    fprintf(file, "%s", Id_toString(s->name));
    fprintf(file, "%s", " = \"");
    fprintf(file, "%s", s->value);
    fprintf(file, "%s", "\";\n");
    return file;
}


/////////////////////////////////////////////////////
// object info
J Machine_ObjInfo_new(List_t offsets) {
    J m;

    Mem_NEW(m);
    m->offsets = offsets;
    return m;
}

static int frameIndex = 1;

File_t Machine_ObjInfo_print(File_t file, J m) {
    List_t p;

    assert(m);
    fprintf(file, "%s", "int [] ");
    fprintf(file, "%s", "layout_");
    fprintf(file, "%d", frameIndex);
    fprintf(file, "%s", " = {");
    //fprintf (file, "%d", m->size);
    fprintf(file, "%s", "; ");
    p = List_getFirst(m->offsets);
    while (p) {
        fprintf(file, "%ld", (long) p->data);
        if (p->next)
            fprintf(file, "%s", ", ");
        p = p->next;
    }
    fprintf(file, "%s", "};\n");
    return file;
}

/////////////////////////////////////////////////////
// frame info
I Machine_FrameInfo_new(List_t offsets, List_t decOffsets, int size) {
    I p;

    Mem_NEW(p);
    p->frameOffsets = offsets;
    p->frameOffsetsDec = decOffsets;
    p->size = size;
    return p;
}


File_t Machine_FrameInfo_print(File_t file, I p) {
    List_t tmp;

    tmp = List_getFirst(p->frameOffsets);
    fprintf(file, "%s", "int[] argOffsets_");
    fprintf(file, "%d = { ;", frameIndex);
    while (tmp) {
        long off = (long) tmp->data;
        fprintf(file, "%ld", off);
        if (tmp->next)
            fprintf(file, "%s", ", ");
        tmp = tmp->next;
    }
    fprintf(file, "%s", "};\n");

    tmp = List_getFirst(p->frameOffsetsDec);
    fprintf(file, "%s", "int[] decOffsets_");
    fprintf(file, "%d = { ;", frameIndex);
    while (tmp) {
        long off = (long) tmp->data;
        fprintf(file, "%ld, ", off);
        tmp = tmp->next;
    }
    fprintf(file, "%s", "};\n\n");
    return file;
}

///////////////////////////////////////////////////////
// program
P Machine_Prog_new(List_t strings, List_t frameInfo, List_t layoutInfo, List_t classes, List_t funcs) {
    P p;

    Mem_NEW(p);
    p->strings = strings;
    p->frameInfo = frameInfo;
    p->layoutInfo = layoutInfo;
    p->classes = classes;
    p->funcs = funcs;
    return p;
}

File_t Machine_Prog_print(File_t file, P x) {
    List_t frame;
    List_t layouts;

    assert(file);
    assert(x);

    fprintf(file, "%s", "///////////////////////////strings:\n");
    List_foldl(x->strings, file, (Poly_tyFold) Machine_Str_print);
    fprintf(file, "%s", "\n");

    // frames
    fprintf(file, "%s", "///////////////////////////frames:\n");
    frameIndex = 1;
    frame = List_getFirst(x->frameInfo);
    while (frame) {
        I i = (I) frame->data;
        Machine_FrameInfo_print(file, i);

        frameIndex++;
        frame = frame->next;
    }
    frameIndex = 1;
    frame = List_getFirst(x->frameInfo);
    fprintf(file, "%s", "frameInfo = {\n");
    while (frame) {
        I i = (I) frame->data;
        fprintf(file, "\t{%s", "argOffsets_");
        fprintf(file, "%d, ", frameIndex);
        fprintf(file, "%s", "decOffsets_");
        fprintf(file, "%d, ", frameIndex);
        fprintf(file, "%d}", i->size);
        if (frame->next)
            fprintf(file, "%s", ",");
        fprintf(file, "%s", "\n");

        frameIndex++;
        frame = frame->next;
    }
    fprintf(file, "%s", "};\n");

    // object layout
    fprintf(file, "%s", "///////////////////////////layouts:\n");
    frameIndex = 1;
    layouts = List_getFirst(x->layoutInfo);
    while (layouts) {
        J j = (J) layouts->data;
        Machine_ObjInfo_print(file, j);

        frameIndex++;
        layouts = layouts->next;
    }
    frameIndex = 1;
    layouts = List_getFirst(x->layoutInfo);
    fprintf(file, "%s", "layoutInfo = {\n");
    while (layouts) {
        //        J j = (J) layouts->data;

        fprintf(file, "\t{%s", "layout_");
        fprintf(file, "%d", frameIndex);
        fprintf(file, "%s", "},\n");

        frameIndex++;
        layouts = layouts->next;
    }
    fprintf(file, "%s", "};\n");

    fprintf(file, "%s", "///////////////////////////classes:\n");
    List_foldl(x->classes, file, (Poly_tyFold) Class_print);
    fprintf(file, "%s", "///////////////////////////functions:\n");
    List_foldl(x->funcs, file, (Poly_tyFold) Machine_Fun_print);
    return file;
}

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
