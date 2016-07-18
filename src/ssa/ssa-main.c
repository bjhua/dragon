#include "../control/pass.h"
#include "type-check.h"
#include "dead-block.h"
#include "trans-ssa.h"
#include "trivial-block.h"
#include "union-block.h"
#include "construct-ssa.h"
#include "const-and-dead.h"
#include "out-ssa.h"
#include "ssa-main.h"

Machine_Prog_t Ssa_main (Ssa_Prog_t p)
{
  Pass_t typeCheck
    , deadBlock
    , trivialBlock
    , unionBlock
    , makeSsa
    , constAndDead
    , outSsa
    , trans;
  Machine_Prog_t q;

  typeCheck = Pass_new ("type-check"
                        , VERBOSE_SUBPASS
                        , p
                        , (Poly_tyId)Ssa_typeCheck);
  p = Pass_doit (&typeCheck);
  
  // want to run this first for it simplifies
  // many later phases by cutting dead blocks
  deadBlock = Pass_new ("deadBlock"
                        , VERBOSE_SUBPASS
                        , p
                        , (Poly_tyId)Ssa_deadBlock);
  p = Pass_doit (&deadBlock);

  trivialBlock = Pass_new ("trivialBlock"
                           , VERBOSE_SUBPASS
                           , p
                           , (Poly_tyId)Ssa_trivialBlock);
  p = Pass_doit (&trivialBlock);

  unionBlock = Pass_new ("unionBlock"
                         , VERBOSE_SUBPASS
                         , p
                         , (Poly_tyId)Ssa_unionBlock);
  p = Pass_doit (&unionBlock);

  makeSsa = Pass_new ("consSsa"
                      , VERBOSE_SUBPASS
                      , p
                      , (Poly_tyId)Ssa_constructSsa);
  p = Pass_doit (&makeSsa);

  constAndDead = Pass_new ("constAndDead"
                           , VERBOSE_SUBPASS
                           , p
                           , (Poly_tyId)Ssa_constAndDead);
  p = Pass_doit (&constAndDead);

  outSsa = Pass_new ("outSsa"
                     , VERBOSE_SUBPASS
                     , p
                     , (Poly_tyId)Ssa_outSsa);
  p = Pass_doit (&outSsa);
  
  trans = Pass_new ("consMachine"
                    , VERBOSE_SUBPASS
                    , p
                    , (Poly_tyId)Trans_ssa);
  q = Pass_doit (&trans);
  
  return q;
}
