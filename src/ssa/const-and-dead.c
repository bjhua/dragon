#include "../lib/trace.h"
#include "../control/log.h"
#include "const-prop.h"
#include "dead-code.h"
#include "const-and-dead.h"

static Ssa_Prog_t Ssa_constAndDeadTraced(Ssa_Prog_t p) {
    p = Ssa_constProp(p);
    p = Ssa_deadCode(p);
    return p;
}

static void printArg(Ssa_Prog_t p) {
    File_t file = File_open("constAndDead.arg", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

static void printResult(Ssa_Prog_t p) {
    File_t file = File_open("constAndDead.result", "w+");
    Ssa_Prog_print(file, p);
    File_close(file);
    return;
}

Ssa_Prog_t Ssa_constAndDead(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Log_POS();

    Trace_TRACE("Ssa_constAndDead", Ssa_constAndDeadTraced, (p), printArg, r, printResult);
    return r;
}
