#include "../control/pass.h"
#include "hil.h"
#include "dead-code.h"
#include "../ssa/ssa.h"
#include "trans-hil.h"
#include "hil-main.h"

Ssa_Prog_t Hil_main (Hil_Prog_t p)
{
  Pass_t deadCode, trans;
  Hil_Prog_t p1;
  Ssa_Prog_t q;

  deadCode = Pass_new ("hilDeadCode",
                       VERBOSE_SUBPASS,
                       p,
                       (Poly_tyId)Hil_deadCode);
  p1 = Pass_doit (&deadCode);  

  trans = Pass_new ("convSsa",
                    VERBOSE_SUBPASS,
                    p1,
                    (Poly_tyId)Hil_trans);
  q = Pass_doit (&trans);

  return q;
}
