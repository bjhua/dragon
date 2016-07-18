#ifndef SSA_UNION_BLOCK_H
#define SSA_UNION_BLOCK_H

#include "ssa.h"

// This code union blocks of this form:
/*
     L1:
        stms1
        jump L2

     L2:
        stms2
        jump L3

============>
     L1:
       stms1
       stms2
       jump L3
 */
// L2 should only have one predessor L1, and L1 has one
// successor L2
Ssa_Prog_t Ssa_unionBlock (Ssa_Prog_t p);

#endif
