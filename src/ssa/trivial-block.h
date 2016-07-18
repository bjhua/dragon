#ifndef SSA_TRIVIAL_BLOCK_H
#define SSA_TRIVIAL_BLOCK_H

#include "ssa.h"

// a trivial block is of the form:
/*
     L1:
        jump L2
 */
// Then all jumps to L1 can be replaced with a jump to L2
Ssa_Prog_t Ssa_trivialBlock (Ssa_Prog_t p);

#endif
