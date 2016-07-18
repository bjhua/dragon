#ifndef SSA_DEAD_BLOCK_H
#define SSA_DEAD_BLOCK_H

#include "ssa.h"

// cutting blocks unreachable from the entry block
Ssa_Prog_t Ssa_deadBlock (Ssa_Prog_t p);

#endif
