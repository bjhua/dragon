#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/property.h"
#include "../lib/trace.h"
#include "dead-block.h"

static Property_t visited = 0;

static void visitBlock (Ssa_Block_t b)
{
  Assert_ASSERT(b);

  Property_set (visited
                , b
                , (Poly_t)1);
  return;
}

//////////////////////////////////////////////////////
static List_t transBlocks (List_t blocks)
{
  List_t tmp, result = List_new ();
  
  Assert_ASSERT(blocks);
  
  tmp = List_getFirst (blocks);
  while (tmp){
    Ssa_Block_t b = (Ssa_Block_t)tmp->data;

    if (Property_get (visited, b))
      List_insertLast (result, b);
    tmp = tmp->next;
  }
  return result;
}

//////////////////////////////////////////////////////
// functions
static Ssa_Fun_t transFunEach (Ssa_Fun_t f)
{
  Ssa_Block_t eb;    // entry block
  List_t newBlocks;
  Graph_t g;

  Assert_ASSERT(f);

  // make a flow graph
  g = Ssa_Fun_toGraph (f);

  // dfs it
  eb = Ssa_Fun_searchLabel (f, f->entry);
  if (!eb)
    Error_impossible ();

  Graph_dfs (g, eb, (Poly_tyVoid)visitBlock);

  // now the properties should be properly set
  newBlocks = transBlocks (f->blocks);
  
  return Ssa_Fun_new (f->type
                      , f->name
                      , f->args
                      , f->decs
                      , newBlocks
                      , f->retId
                      , f->entry
                      , f->exitt);
}


//////////////////////////////////////////////////////
// programs
static Ssa_Prog_t Ssa_deadBlockTraced (Ssa_Prog_t p)
{
  List_t newFuncs;

  Assert_ASSERT(p);

  // init the property
  visited = Property_new 
    ((Poly_tyPlist)Ssa_Block_plist);

  newFuncs = List_map (p->funcs, (Poly_tyId)transFunEach);

  // clear property
  Property_clear (visited);

  return Ssa_Prog_new (p->classes, newFuncs);
}

/////////////////////////////////////////////////////
// main functions
static void printArg (Ssa_Prog_t p)
{
  File_saveToFile ("ssa_deadBlock.arg"
                   , (Poly_tyPrint)Ssa_Prog_print
                   , p);
  Ssa_Prog_toDot (p, "beforeSsaDeadBlock");
  return;
}

static void printResult (Ssa_Prog_t p)
{
  File_saveToFile ("ssa_deadBlock.result"
                   , (Poly_tyPrint)Ssa_Prog_print
                   , p);
  Ssa_Prog_toDot (p, "afterSsaDeadBlock");
  return;
}

Ssa_Prog_t Ssa_deadBlock (Ssa_Prog_t p)
{
  Ssa_Prog_t r;

  Trace_TRACE("Ssa_deadBlock"
              , Ssa_deadBlockTraced
              , (p)
              , printArg
              , r
              , printResult);
  return r;
}
