#include "../control/pass.h"
#include "gen-frame.h"
#include "gen-layout.h"
#include "machine-main.h"

Machine_Prog_t Machine_main(Machine_Prog_t p) {
    Pass_t genFrame, genLayout;

    genFrame = Pass_new("genFrame", VERBOSE_SUBPASS, p, (Poly_tyId) Machine_genFrame);
    p = Pass_doit(&genFrame);

    genLayout = Pass_new("genLayout", VERBOSE_SUBPASS, p, (Poly_tyId) Machine_genLayout);
    p = Pass_doit(&genLayout);

    return p;
}
