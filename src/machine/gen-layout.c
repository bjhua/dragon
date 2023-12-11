#include "gen-layout.h"
#include "../control/control.h"
#include "../control/log.h"
#include "../lib/error.h"
#include "../lib/list.h"
#include "../lib/property.h"
#include "../lib/trace.h"
#include "runtime.h"
#include <assert.h>

// Generate object layout information for each class.

// Map every class name to its index in the layouts.
// Class_Name_t -> int
static Property_t indexProp = 0;

// Map every class field to its offsets (starting from 0)
static Property_t fieldProp = 0;

// Map every class name to its size.
// Class_Name_t -> int
static Property_t sizeProp = 0;


//
static Machine_Mem_t genMem(Machine_Mem_t m) {
    assert(m);
    switch (m->kind) {
        case MACHINE_MEM_ARRAY:
            return m;
        case MACHINE_MEM_CLASS: {
            long index = (long) Property_get(fieldProp, m->u.class.field);
            return Machine_Mem_new_class(m->u.class.name, m->u.class.field, index);
        }
        default:
            Error_impossible();
            return 0;
    }
    Error_impossible();
    return 0;
}

//
static Machine_Stm_t genStm(Machine_Stm_t s) {
    switch (s->kind) {
        case MACHINE_STM_STORE: {
            return Machine_Stm_new_store(genMem(s->u.store.m), s->u.store.src);
        }
        case MACHINE_STM_LOAD:
            return Machine_Stm_new_load(s->u.load.dest, genMem(s->u.load.m));
        case MACHINE_STM_NEW_CLASS: {
            long index = (long) Property_get(indexProp, s->u.newClass.cname);
            long size = (long) Property_get(sizeProp, s->u.newClass.cname);

            return Machine_Stm_Runtime_class(s->u.newClass.dest, index, size, Runtime_class);
        }
        case MACHINE_STM_NEW_ARRAY: {
            // the "atype" should also have 3 cases:
            Atype_t ty = s->u.newArray.ty;

            switch (ty->kind) {
                case ATYPE_INT:
                    return Machine_Stm_Runtime_array(s->u.newArray.dest, 0, s->u.newArray.size, Control_Target_size, Runtime_array);
                case ATYPE_STRING:
                    return Machine_Stm_Runtime_array(s->u.newArray.dest, 0, s->u.newArray.size, Control_Target_size, Runtime_array);
                case ATYPE_CLASS:
                    return Machine_Stm_Runtime_array(s->u.newArray.dest, 1, s->u.newArray.size, Control_Target_size, Runtime_array);
                default:
                    Error_impossible();
                    return 0;
            }
        }
        default:
            return s;
    }
    Error_impossible();
    return 0;
}

//
static Machine_Block_t genBlock(Machine_Block_t b) {
    List_t newStms;

    newStms = List_map(b->stms, (Poly_tyId) genStm);
    return Machine_Block_new(b->label, newStms, b->transfer);
    ;
}

static Machine_Fun_t genFunc(Machine_Fun_t f) {
    List_t newBlocks;

    newBlocks = List_map(f->blocks, (Poly_tyId) genBlock);

    return Machine_Fun_new(f->type, f->name, f->args, f->decs, newBlocks, f->retId, f->entry, f->exitt, f->frameIndex);
}

// each class's index into the table
static long layoutIndex = 0;

static Machine_ObjInfo_t genLayout(Class_t class) {
    int offset = 1;
    List_t offsets = List_new();
    Id_t name;
    List_t decs;
    long num = 0;

    assert(class);

    name = class->name;
    decs = class->decs;

    // remember this class's index
    layoutIndex++;
    Property_set(indexProp, name, (Poly_t) layoutIndex);

    decs = List_getFirst(decs);
    while (decs) {
        Dec_t dec = (Dec_t) decs->data;
        Atype_t ty = dec->ty;

        // remember this field's offset in this class
        Property_set(fieldProp, dec->id, (Poly_t) num);

        // this is a pointer
        if (Atype_maybeGc(ty)) {
            long i = Control_Target_size * offset;

            List_insertLast(offsets, (Poly_t) i);
            offset++;
        }
        num++;
        decs = decs->next;
    }
    num *= Control_Target_size;
    Property_set(sizeProp, name, (Poly_t) num);
    return Machine_ObjInfo_new(offsets);
}

static Machine_Prog_t
Machine_genLayoutTraced(Machine_Prog_t p) {
    List_t newFuncs, layouts;

    Runtime_init();

    indexProp = Property_new((Poly_tyPlist) Id_plist);
    fieldProp = Property_new((Poly_tyPlist) Id_plist);
    sizeProp = Property_new((Poly_tyPlist) Id_plist);


    // generate layout information for each class
    layouts = List_map(p->classes, (Poly_tyId) genLayout);

    newFuncs = List_map(p->funcs, (Poly_tyId) genFunc);

    Property_clear(indexProp);
    Property_clear(fieldProp);
    Property_clear(sizeProp);

    Log_str("gen layout finished:");

    return Machine_Prog_new(p->strings, p->frameInfo, layouts, p->classes, newFuncs);
}

static void printArg(Machine_Prog_t p) {
    File_saveToFile("genLayout.arg", (Poly_tyPrint) Machine_Prog_print, p);
    return;
}

static void printResult(Machine_Prog_t p) {
    File_saveToFile("genLayout.result", (Poly_tyPrint) Machine_Prog_print, p);
    return;
}

Machine_Prog_t Machine_genLayout(Machine_Prog_t p) {
    Machine_Prog_t r;

    Trace_TRACE("genLayout", Machine_genLayoutTraced, (p), printArg, r, printResult);
    return r;
}
