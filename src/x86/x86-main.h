#ifndef X86_MAIN_H
#define X86_MAIN_H

#include "../lib/tuple.h"
#include "../machine/machine.h"
#include "../x86/x86.h"

/* a tuple <IR, asm> of the IR and its assembly */
Tuple_t X86_main(Machine_Prog_t p);

#endif
