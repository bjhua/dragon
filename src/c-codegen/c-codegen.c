#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include "../control/control.h"
#include "c-codegen.h"

//////////////////////////////////////////////////////
//
static File_t file = 0;
// to index the frames and layouts.
static int index = 1;
//
static String_t argOffsets = "argOffsets";
static String_t decOffsets = "decOffsets";
// 
static String_t layouts = "layouts";
//
static Label_t currentLabel = 0;

//////////////////////////////////////////////////////
// headers
static void outputHeaders() {
    return;
    //fprintf (file, "%s", "#include <main-header.h>");
}


//////////////////////////////////////////////////////
// strings
static void outputOneString(Machine_Str_t s) {
    char c;
    char *a = s->value;

    Assert_ASSERT(s->name);
    fprintf(file, "static char *%s = ", Id_toString(s->name));
    fprintf(file, "%s", "\"");
    while ((c = *a++)) {
        switch (c) {
            case '\n':
                fprintf(file, "%s", "\\n");
                break;
            case '\t':
                fprintf(file, "%s", "\\t");
                break;
            default:
                fprintf(file, "%c", c);
                break;
        }
    }
    fprintf(file, "%s", "\";\n");
}

static void outputStrings(List_t strings) {
    List_foreach(strings, (Poly_tyVoid) outputOneString);
    return;
}

//////////////////////////////////////////////////////
// frames
static void outputFrame(Machine_FrameInfo_t p) {
    return;

    List_t tmp;

    tmp = List_getFirst(p->frameOffsets);
    fprintf(file, "int %s", argOffsets);
    fprintf(file, "_%d[] = {%d ", index, List_size(p->frameOffsets));
    while (tmp) {
        long off = (long) tmp->data;
        fprintf(file, ", %ld", off);
        tmp = tmp->next;
    }
    fprintf(file, "%s", "};\n");

    tmp = List_getFirst(p->frameOffsetsDec);
    fprintf(file, "int %s", decOffsets);
    fprintf(file, "_%d[] = {%d ", index, List_size(p->frameOffsetsDec));
    while (tmp) {
        long off = (long) tmp->data;
        fprintf(file, ", %ld", off);
        tmp = tmp->next;
    }
    fprintf(file, "%s", "};\n\n");
    return;
}

/****************************************************/
// local exception handlers
// List<Label_t>
// For now, this is a dirty hack, the handler should
// be take down from the source by a "catch" clause.
static List_t handlers = 0;

//////////////////////////////////////////////////////
// layouts
static void outputLayouts(Machine_ObjInfo_t m) {
    return;
    List_t p;

    Assert_ASSERT(m);
    fprintf(file, "%s", "int ");
    fprintf(file, "%s", "layout_");
    fprintf(file, "%d[]", index);
    fprintf(file, "%s", " = {");
    fprintf(file, "%d", List_size(m->offsets));
    p = List_getFirst(m->offsets);
    while (p) {
        fprintf(file, ", %ld", (long) p->data);
        p = p->next;
    }
    fprintf(file, "%s", "};\n");
    return;
}

//////////////////////////////////////////////////////
// atype
static void outputAtype(Atype_t ty) {
    Assert_ASSERT (ty);
    switch (ty->kind) {
        case ATYPE_INT:
            fprintf(file, "%s", "int");
            return;
        case ATYPE_INT_ARRAY:
            fprintf(file, "%s", "int *");
            return;
        case ATYPE_STRING:
            fprintf(file, "%s", "char *");
            return;
        case ATYPE_STRING_ARRAY:
            fprintf(file, "%s", "char **");
            return;
        case ATYPE_CLASS:
            fprintf(file, "%s", "void *");
            return;
        case ATYPE_CLASS_ARRAY:
            fprintf(file, "%s", "void **");
            return;
        case ATYPE_FUN:
            Error_impossible ();
            return;
        default:
            Error_impossible ();
            return;
    }
    Error_impossible ();
    return;
}

//////////////////////////////////////////////////////
// dec
static void outputDec(Dec_t d) {
    outputAtype(d->ty);
    fprintf(file, "%s", " ");
    fprintf(file, "%s", Id_toString(d->id));
    return;
}

//////////////////////////////////////////////////////
// args
static void outputArgs(List_t l) {
    l = List_getFirst(l);
    while (l) {
        Dec_t dec = (Dec_t) l->data;
        outputDec(dec);
        if (l->next)
            fprintf(file, "%s", ", ");
        l = l->next;
    }
    return;
}

//////////////////////////////////////////////////////
// decs
static void outputDecs(List_t l) {
    l = List_getFirst(l);
    while (l) {
        Dec_t dec = (Dec_t) l->data;
        fprintf(file, "%s", "  ");
        outputDec(dec);
        fprintf(file, "%s", ";\n");
        l = l->next;
    }
    return;
}

//////////////////////////////////////////////////////
// operand
static void outputOperand(Machine_Operand_t o) {
    Assert_ASSERT(o);
    switch (o->kind) {
        case MACHINE_OP_INT:
            fprintf(file, "%d", o->u.intlit);
            break;
        case MACHINE_OP_GLOBAL:
            fprintf(file, "%s", Id_toString(o->u.id));
            break;
        case MACHINE_OP_ID:
            fprintf(file, "%s", Id_toString(o->u.id));
            break;
        default:
            Error_impossible ();
            break;
    }
    return;
}

//////////////////////////////////////////////////////
// memory
static void outputMem(Machine_Mem_t m) {
    Assert_ASSERT(m);
    switch (m->kind) {
        case MACHINE_MEM_ARRAY:
            fprintf(file, "*(((A)%s", Id_toString(m->u.array.name));
            fprintf(file, "%s", ")+");
            outputOperand(m->u.array.index);
            fprintf(file, "%s", ")");
            break;
        case MACHINE_MEM_CLASS:
            fprintf(file, "*(((C)%s", Id_toString(m->u.class.name));
            // the index should always be equal or greater than 0,
            // so the "+" here is of no problem.
            fprintf(file, "%s", ")+");
            fprintf(file, "%d", (m->u.class.index));
            fprintf(file, "%s", ")");
            break;
        default:
            Error_impossible ();
            break;
    }
    return;
}

//////////////////////////////////////////////////////
// stm
static void outputStm(Machine_Stm_t s) {
    Assert_ASSERT(s);
    fprintf(file, "%s", "  ");
    switch (s->kind) {
        case MACHINE_STM_MOVE:
            fprintf(file, "%s", Id_toString(s->u.move.dest));
            fprintf(file, "%s", " = ");
            outputOperand(s->u.move.src);
            break;
        case MACHINE_STM_BOP:
            fprintf(file, "%s", Id_toString(s->u.bop.dest));
            fprintf(file, "%s", " = ");
            outputOperand(s->u.bop.left);
            fprintf(file, "%s", Operator_toString(s->u.bop.op));
            outputOperand(s->u.bop.right);
            break;
        case MACHINE_STM_UOP:
            fprintf(file, "%s", Id_toString(s->u.uop.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Operator_toString(s->u.uop.op));
            outputOperand(s->u.uop.src);
            break;

        case MACHINE_STM_STORE:
            outputMem(s->u.store.m);
            fprintf(file, "%s", " = ");
            outputOperand(s->u.store.src);
            break;
        case MACHINE_STM_LOAD:
            fprintf(file, "%s", Id_toString(s->u.load.dest));
            fprintf(file, "%s", " = ");
            outputMem(s->u.load.m);
            break;
        case MACHINE_STM_TRY: {
            List_insertLast(handlers, s->u.try);
            fprintf(file, "do{\n  extern int %s%s;\n  ", Label_toString(s->u.try), Label_toString(s->u.try));
            fprintf(file, "Dragon_Exn_try((int)&%s%s);\n  }while(0)", Label_toString(s->u.try),
                    Label_toString(s->u.try));
            break;
        }
        case MACHINE_STM_TRY_END: {
            // there is a very dirty hack to squell the
            // next goto...
            currentLabel = s->u.tryEnd;
            List_insertLast(handlers, currentLabel);
            fprintf(file, "do{");
            fprintf(file, "extern int %s%s;\n", Label_toString(s->u.tryEnd), Label_toString(s->u.tryEnd));
            fprintf(file, "Dragon_Exn_end ((int)&%s%s);", Label_toString(s->u.tryEnd), Label_toString(s->u.tryEnd));
            fprintf(file, "}while (0)");
            break;
        }
        case MACHINE_STM_RUNTIME_CLASS: {
            fprintf(file, "%s", Id_toString(s->u.class.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(s->u.class.fname));

            fprintf(file, "%s", " (");
            fprintf(file, "%d, ", s->u.class.index);
            fprintf(file, "%d", s->u.class.size);
            fprintf(file, "%s", ")");
            fprintf(file, ";%s", " // (index, size)");
            break;
        }
        case MACHINE_STM_RUNTIME_ARRAY: {
            fprintf(file, "%s", Id_toString(s->u.array.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(s->u.array.fname));
            fprintf(file, "%s", "(");
            fprintf(file, "%d", s->u.array.isPtr);
            fprintf(file, "%s", ", ");
            outputOperand(s->u.array.size);
            fprintf(file, "%s", ", ");
            fprintf(file, "%d", s->u.array.scale);
            fprintf(file, "%s", "); // (isPtr, size, scale)");
            break;
        }
        default:
            fprintf(stderr, "%d", s->kind);
            Error_impossible ();
            break;
    }
    fprintf(file, "%s", ";\n");
    return;
}

//////////////////////////////////////////////////////
// transfer
static void outputTransfer(Machine_Transfer_t t) {
    Assert_ASSERT(t);
    fprintf(file, "%s", "  ");
    switch (t->kind) {
        case MACHINE_TRANS_IF:
            fprintf(file, "%s", "if (");
            Machine_Operand_print(file, t->u.iff.cond);
            fprintf(file, "%s", ")\n    goto ");
            fprintf(file, "%s", Label_toString(t->u.iff.truee));
            fprintf(file, "%s", ";\n");
            fprintf(file, "%s", "  else goto ");
            fprintf(file, "%s", Label_toString(t->u.iff.falsee));
            break;
        case MACHINE_TRANS_JUMP:
            if (!currentLabel) {
                fprintf(file, "%s", "goto ");
                fprintf(file, "%s", Label_toString(t->u.jump));
            } else {
                currentLabel = 0;
            }
            break;
        case MACHINE_TRANS_RETURN:
            fprintf(file, "%s", "return ");
            Machine_Operand_print(file, t->u.ret);
            break;
        case MACHINE_TRANS_THROW:
            fprintf(file, "%s", "Dragon_Exn_throw ()");
            break;
        case MACHINE_TRANS_CALL: {
            List_t args;

            // must save
            if (t->u.call.leave)
                fprintf(file, "%s", "Dragon_Exn_save ();\n  ");
            fprintf(file, "%s", Id_toString(t->u.call.dest));
            fprintf(file, "%s", " = ");
            fprintf(file, "%s", Id_toString(t->u.call.name));
            fprintf(file, "%s", "(");
            args = List_getFirst(t->u.call.args);
            while (args) {
                Machine_Operand_t op = (Machine_Operand_t) args->data;

                outputOperand(op);
                if (args->next)
                    fprintf(file, "%s", ", ");
                args = args->next;
            }
            fprintf(file, "%s", ")");
            break;
        }
        case MACHINE_TRANS_CALL_NOASSIGN: {
            List_t args;

            // must save
            if (t->u.call.leave)
                fprintf(file, "%s", "Dragon_Exn_save ();\n  ");
            fprintf(file, "%s", Id_toString(t->u.call.name));
            fprintf(file, "%s", "(");
            args = List_getFirst(t->u.call.args);
            while (args) {
                Machine_Operand_t op = (Machine_Operand_t) args->data;

                outputOperand(op);
                if (args->next)
                    fprintf(file, "%s", ", ");
                args = args->next;
            }
            fprintf(file, "%s", ")");
            break;
        }
        default:
            Error_impossible ();
            break;
    }
    fprintf(file, "%s", ";\n");
    return;
}

//////////////////////////////////////////////////////
// block
static void outputBlock(Machine_Block_t b) {
    Assert_ASSERT(b);

    fprintf(file, "%s:\n", Label_toString(b->label));
    // if this label appears in the "handlers", it
    // says that it is a handler...
    if (List_exists(handlers, b->label, (Poly_tyEquals) Label_equals))
        fprintf(file, "__asm__(\"_%s%s:\\n\\t.byte 0x90\\n\");\n", Label_toString(b->label), Label_toString(b->label));
    List_foreach(b->stms, (Poly_tyVoid) outputStm);
    outputTransfer(b->transfer);
    fprintf(file, "%s", "\n");
    return;
}

//////////////////////////////////////////////////////
// function
static void outputFunction(Machine_Fun_t f) {
    Assert_ASSERT(f);

    handlers = List_new();
    outputAtype(f->type);
    fprintf(file, " %s (", Id_toString(f->name));
    outputArgs(f->args);
    fprintf(file, "%s", ")\n{\n");
    outputDecs(f->decs);
    fprintf(file, "\n");

    List_foreach(f->blocks, (Poly_tyVoid) outputBlock);
    // fprintf (file, "%s = %d", "FRAME_INDEX ", f->frameIndex);
    // to shut up the C compilers
    //fprintf (file, "  %s", "return 0;\n");
    fprintf(file, "%s", "}\n\n");
    return;
}


//////////////////////////////////////////////////////
// programs

#define COMMENT(s)\
  "\n//////////////////////////////////////////////////\n"        \
  "// " s "\n"

static Machine_Prog_t C_codegenTraced(Machine_Prog_t p) {
    List_t frame;
    List_t layouts;

    Assert_ASSERT(p);
    fprintf(file, "%s", COMMENT("header files:\n"));
    fprintf(file, "%s", "extern long printi(long);\n\n");

    fprintf(file, "%s", COMMENT("headers:"));
    outputHeaders();
    fprintf(file, "%s", "\n\n");


    fprintf(file, "%s", COMMENT("strings:"));
    outputStrings(p->strings);
    fprintf(file, "%s", "\n\n");

#if 0

    fprintf(file, "%s", COMMENT("frames:"));
    index = 1;
    frame = List_getFirst(p->frameInfo);
    while (frame) {
        Machine_FrameInfo_t i
                = (Machine_FrameInfo_t) frame->data;

        outputFrame(i);
        index++;
        frame = frame->next;
    }
    index = 1;
    frame = List_getFirst(p->frameInfo);
    fprintf(file, "%s", "int frameInfo[");
    fprintf(file, "%d", List_size(p->frameInfo));
    fprintf(file, "%s", "][3] = {\n");
    while (frame) {
        Machine_FrameInfo_t i
                = (Machine_FrameInfo_t) frame->data;
        fprintf(file, "\t{(int)%s", argOffsets);
        fprintf(file, "_%d, ", index);
        fprintf(file, "(int)%s", decOffsets);
        fprintf(file, "_%d, ", index);
        fprintf(file, "%d}", i->size);
        if (frame->next)
            fprintf(file, "%s", ",");
        fprintf(file, "%s", "\n");

        index++;
        frame = frame->next;
    }
    fprintf(file, "%s", "};\n");

    // layouts
    fprintf(file, "%s", COMMENT("layouts:"));
    index = 1;
    layouts = List_getFirst(p->layoutInfo);
    while (layouts) {
        Machine_ObjInfo_t j = (Machine_ObjInfo_t) layouts->data;
        outputLayouts(j);

        index++;
        layouts = layouts->next;
    }
    index = 1;
    layouts = List_getFirst(p->layoutInfo);
    fprintf(file, "int %s", "layoutInfo[");
    fprintf(file, "%d", List_size(p->layoutInfo));
    fprintf(file, "%s", "][1] = {\n");
    while (layouts) {
        fprintf(file, "\t{(int)%s", "layout_");
        fprintf(file, "%d", index);
        fprintf(file, "%s", "}");
        if (layouts->next)
            fprintf(file, "%s", ",");
        fprintf(file, "%s", "\n");
        index++;
        layouts = layouts->next;
    }
    fprintf(file, "%s", "};\n");
#endif

    fprintf(file, "%s", COMMENT("functions:"));
    List_foreach(p->funcs, (Poly_tyVoid) outputFunction);

    return p;
}

static void outArg(Machine_Prog_t p) {
    File_saveToFile("c-codegen.arg", (Poly_tyPrint) Machine_Prog_print, p);
    return;
}

static void outResult(Machine_Prog_t p) {
    File_t file = File_open("c-codegen.result", "w+");
    File_write(file, "leave empty!");
    File_close(file);
    return;
}

int C_codegen(File_t f, Machine_Prog_t p) {
    Machine_Prog_t r;

    file = f;
    Trace_TRACE("C_codegen"
                , C_codegenTraced
                , (p)
                , outArg
                , r
                , outResult);
    return 0;
}
