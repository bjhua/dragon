#ifndef SSA_CONST_AND_DEAD_H
#define SSA_CONST_AND_DEAD_H

// This pass consists of many passes, which does
// any constant propagation and constant folding, as well
// as any dead code (dec) elimination.
// This pass should run when any other optimization is
// done.

#include "ssa.h"

Ssa_Prog_t Ssa_constAndDead(Ssa_Prog_t p);

#endif
