#include "gen-frame.h"
#include "../control/control.h"
#include "../lib/list.h"
#include "../lib/trace.h"

// Generate frame information for each function.
// For now, assume all arguments and local declarations
// are located on the stack. (change this later!)

static List_t allOffsets = 0;
// the 0 index is the "length" info
static int index = 1;

static List_t getFrameInfo(void) {
    List_t tmp = allOffsets;
    allOffsets = 0;
    return tmp;
}

//
static int genOffsets(List_t args, List_t decs) {
    Machine_FrameInfo_t newInfo;
    List_t offsets = List_new();
    List_t decOffsets = List_new();
    int size = 0;
    int argindex = -2;
    int decindex = 1;
    //    int num = 0;

    args = List_getFirst(args);
    while (args) {
        Dec_t dec = (Dec_t) args->data;
        // increment the total frame size
        size++;
        // if the arg is an reference variable, then record it
        if (Dec_maybeGc(dec)) {
            long offset = argindex * Control_Target_size;
            List_insertLast(offsets, (Poly_t) offset);
            argindex--;
        }
        args = args->next;
    }
    decs = List_getFirst(decs);
    while (decs) {
        Dec_t dec = (Dec_t) decs->data;
        size++;
        if (Dec_maybeGc(dec)) {
            long offset = decindex * Control_Target_size;
            List_insertLast(decOffsets, (Poly_t) offset);
            decindex++;
        }
        decs = decs->next;
    }

    size *= Control_Target_size;
    newInfo = Machine_FrameInfo_new(offsets, decOffsets, size);
    List_insertLast(allOffsets, newInfo);
    return index++;
}

static Machine_Fun_t genFunc(Machine_Fun_t f) {
    int frameIndex;

    frameIndex = genOffsets(f->args, f->decs);
    return Machine_Fun_new(f->type, f->name, f->args, f->decs, f->blocks, f->retId, f->entry, f->exitt, frameIndex);
}

static Machine_Prog_t Machine_genFrameTraced(Machine_Prog_t p) {
    List_t newFuncs;

    allOffsets = List_new();
    newFuncs = List_map(p->funcs, (Poly_tyId) genFunc);
    return Machine_Prog_new(p->strings, getFrameInfo(), p->layoutInfo, p->classes, newFuncs);
}

static void printArg(Machine_Prog_t p) {
    File_t file = File_open("genFrame.arg", "w+");
    Machine_Prog_print(file, p);
    File_close(file);
}

static void printResult(Machine_Prog_t p) {
    File_t file = File_open("genFrame.result", "w+");
    Machine_Prog_print(file, p);
    File_close(file);
}

Machine_Prog_t Machine_genFrame(Machine_Prog_t p) {
    Machine_Prog_t r;

    Trace_TRACE("genFrame", Machine_genFrameTraced, (p), printArg, r, printResult);
    return r;
}
