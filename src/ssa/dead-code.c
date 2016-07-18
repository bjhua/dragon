#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include "../lib/list.h"
#include "../control/log.h"
#include "dead-code.h"

///////////////////////////////////////////////////////
// This module eliminates dead code and dead (local)
// declarations.


// whether or not we should do dead code elimination.
static int flag = 0;
// whether or not find more dead code in one pass.
static int founddead = 0;

static void mark ()
{
  Log_str ("mark() is called\n");
  flag = 1;
  founddead = 1;
}

static void clear ()
{
  founddead = 0;
}


enum Dead_t{
  UNKNOWN = 0,   
  DEAD = 1,
  NOTDEAD = 2
};


// Id_t -> int
// how many times an id is used, return 0 by default
static Property_t usedProp = 0;

// Id_t -> Dead_t
// whether or not an id is dead, return UNKNOWN by default
static Property_t deadProp = 0;


//////////////////////////////////////////////////////
// analyzing used vars

static void incUse (Id_t id)
{
  long r;

  // when this property has not been set on that var, the
  // "get" will return 0 (rather than the number 0).
  // We abuse it.
  r = (long)Property_get (usedProp, id);
  r++;
  Property_set (usedProp, id, (Poly_t)r);
  Log_strs ("\n"
            , Id_toString (id)
            , " ===> "
            , Int_toString (r)
            , " times"
            , 0);
  return;
}

static void analyzeBlockUsed (Ssa_Block_t b)
{
  Ssa_Block_foreachUse(b
                       , incUse);
}

static void analyzeFunUsed (Ssa_Fun_t f)
{
  List_foreach (f->blocks, (Poly_tyVoid)analyzeBlockUsed);
}

static void analyzeUsed (Ssa_Prog_t p)
{
  List_foreach (p->funcs, (Poly_tyVoid)analyzeFunUsed);
}

//////////////////////////////////////////////////
// analyzing dead code

static int isDead (Id_t id)
{
  return (int)Property_get (deadProp, id) == DEAD;
}

static void markDead (Id_t id)
{
  Property_set (deadProp, id, (Poly_t)DEAD);
}


static int notUsed (Id_t id)
{
  return 0 == Property_get (usedProp, id);
}

static void decUse (Id_t id)
{
  long r = (long)Property_get (usedProp, id);
  if (!r){
    Error_impossible ();
    return;
  }
  --r;
  Property_set (usedProp, id, (Poly_t)r);
  // to catch bugs
  if (!r){
    if (Property_get (deadProp, id)){
      Error_impossible ();
      return;
    }
    // found a new dead var
    mark ();
    Property_set (deadProp, id, (Poly_t)DEAD);
  }
  return;
}

static void analyzeStm (Ssa_Stm_t s)
{
  Assert_ASSERT(s);
  switch (s->kind){
  case SSA_STM_MOVE:{
    if (isDead(s->u.move.dest))
      return;
    
    if (notUsed (s->u.move.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.move.dest);
      
      // all right-hand side decrement
      Ssa_Stm_foreachUse (s, decUse);
    }
    return;
  }
  case SSA_STM_BOP:{
    if (isDead(s->u.bop.dest))
      return;
    
    if (notUsed (s->u.move.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.move.dest);

      Ssa_Stm_foreachUse (s, decUse);
    }
    return;
  }    
  case SSA_STM_UOP:{
    if (isDead(s->u.uop.dest))
      return;
    
    if (notUsed (s->u.move.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.move.dest);

      Ssa_Stm_foreachUse (s, decUse);
    }
    return;
  }
  case SSA_STM_STORE:
    return;
  case SSA_STM_LOAD:{
    if (isDead(s->u.load.dest))
      return;
    
    if (notUsed (s->u.move.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.load.dest);
      Ssa_Stm_foreachUse (s, decUse);
    }
    return;
  }
  case SSA_STM_NEW_CLASS:{
    if (isDead(s->u.newClass.dest))
      return;
    
    if (notUsed (s->u.newClass.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.newClass.dest);
    }
    return;
  }
  case SSA_STM_NEW_ARRAY:{
    if (isDead(s->u.newArray.dest))
      return;
    
    if (notUsed (s->u.newArray.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.newArray.dest);
    }
    return;
  }    
  case SSA_STM_PHI:{
    if (isDead(s->u.phi.dest))
      return;
    
    if (notUsed (s->u.phi.dest)){
      Log_strs ("found a dead var: "
                , Id_toString (s->u.move.dest)
                , 0);
      
      mark ();
      markDead (s->u.phi.dest);
      Ssa_Stm_foreachUse (s, decUse);
    }
    return;
  }
    // no interesting information
  case SSA_STM_TRY:
  case SSA_STM_TRY_END:
    return;
  default:
    fprintf (stderr, "%d", s->kind);
    Error_impossible ();
    return;
  }
  Error_impossible (); 
  return;
}

static void analyzeTransfer (Ssa_Transfer_t t)
{
  Assert_ASSERT(t);
  switch (t->kind){
  case SSA_TRANS_IF:
    return;
  case SSA_TRANS_JUMP:
    return;
  case SSA_TRANS_RETURN:
    return;
  case SSA_TRANS_THROW:
    return;
  case SSA_TRANS_CALL:{
    if (t->u.call.dest){
      if (isDead (t->u.call.dest))
        return;

      if (notUsed(t->u.call.dest)){
        Log_strs ("found a dead var: "
                  , Id_toString (t->u.call.dest)
                  , 0);
      
        mark ();
        markDead (t->u.call.dest);
        return;
      }
    }    
    return;
  }
  default:
    Error_impossible ();
    return;
  }
  Error_impossible ();
  return;
}

static void analyzeBlock (Ssa_Block_t b)
{
  List_foreach (b->stms, (Poly_tyVoid)analyzeStm);
  analyzeTransfer (b->transfer);
  return;
}

static void analyzeFun (Ssa_Fun_t f)
{
  List_foreach (f->blocks, (Poly_tyVoid)analyzeBlock);
}

static void analyze (Ssa_Prog_t p)
{
  
  List_foreach (p->funcs, (Poly_tyVoid)analyzeFun);

}

////////////////////////////////////////////////////////
// rewriting

// zero for dead.
static Ssa_Stm_t doit (Ssa_Stm_t s)
{
  Assert_ASSERT(s);
  switch (s->kind){
  case SSA_STM_MOVE:
    if (isDead(s->u.move.dest))
      return 0;
    return s;
  case SSA_STM_BOP:
    if (isDead(s->u.bop.dest))
      return 0;
    return s;
  case SSA_STM_UOP:
    if (isDead(s->u.uop.dest))
      return 0;
    return s;
  case SSA_STM_STORE:
    return s;
  case SSA_STM_LOAD:
    if (isDead(s->u.load.dest))
      return 0;
    return s;
  case SSA_STM_NEW_CLASS:
    if (isDead(s->u.newClass.dest))
      return 0;    
    return s;
  case SSA_STM_NEW_ARRAY:
    if (isDead(s->u.newArray.dest))
      return 0;    
    return s;
  case SSA_STM_PHI:
    if (isDead(s->u.phi.dest))
      return 0;    
    return s;
  case SSA_STM_TRY:
    return s;
  case SSA_STM_TRY_END:
    return s;
  default:
    fprintf (stderr, "%d", s->kind);
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

static Ssa_Transfer_t rewriteTransfer (Ssa_Transfer_t t)
{
  Assert_ASSERT(t);
  switch (t->kind){
  case SSA_TRANS_IF:
    return t;
  case SSA_TRANS_JUMP:
    return t;
  case SSA_TRANS_RETURN:
    return t;
  case SSA_TRANS_THROW:
    return t;
  case SSA_TRANS_CALL:{
    if (t->u.call.dest){
      if (isDead (t->u.call.dest)){
        printf ("found dead: %s\n", Id_toString (t->u.call.dest));
        return Ssa_Transfer_new_call (0
                                      , t->u.call.name
                                      , t->u.call.args
                                      , t->u.call.leave
                                      , t->u.call.normal);
      }
    }    
    return t;
  }
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

static Ssa_Block_t rewriteBlock (Ssa_Block_t b)
{  
  List_t newStms = List_new ();
  List_t tmp = List_getFirst (b->stms);
  Ssa_Transfer_t newTrans;

  while (tmp){
    Ssa_Stm_t s = doit (tmp->data);
    if (s)
      List_insertLast (newStms, s);
    tmp = tmp->next;
  }
  newTrans = rewriteTransfer (b->transfer);
  return Ssa_Block_new (b->label
                        , newStms
                        , newTrans);
}

static int decFilter (Dec_t dec)
{
  if (NOTDEAD == (int)Property_get (deadProp, dec->id)){
    Log_strs (Dec_toString (dec)
              , " is NOT dead\n"
              , 0);
    return 1;
  }
  if ((int)Property_get (deadProp, dec->id) == DEAD
      || (0==Property_get (usedProp, dec->id))){
    Log_strs ("found a dead declaration: "
              , Dec_toString (dec)
              , "\n"
              , 0);
    return 0;
  }
  else{
    Log_str ("what's going on?\n");
  }
  return 1;
}

static Ssa_Fun_t rewriteFun (Ssa_Fun_t f)
{
  List_t newBlocks;
  List_t newDecs;

  newBlocks = List_map (f->blocks
                        , (Poly_tyId)rewriteBlock);

  newDecs = List_filter (f->decs, (Poly_tyPred)decFilter);

  return Ssa_Fun_new (f->type
                      , f->name
                      , f->args
                      , newDecs
                      , newBlocks
                      , f->retId
                      , f->entry
                      , f->exitt);
}

static Ssa_Prog_t rewriteProg (Ssa_Prog_t p)
{
  List_t newfuncs;

  newfuncs = List_map (p->funcs, (Poly_tyId)rewriteFun);
  return Ssa_Prog_new (p->classes, newfuncs);
}

////////////////////////////////////////////////
// program
static Ssa_Prog_t Ssa_deadCodeTraced (Ssa_Prog_t p)
{
  usedProp = Property_new ((Poly_tyPlist)Id_plist);
  deadProp = Property_new ((Poly_tyPlist)Id_plist);
  

  Log_str ("analyzing used var starting:");  
  analyzeUsed (p);
  Log_str ("analyzing used var finished:");

  
  Log_str ("analyzing dead code starting:");
  
  do{
    founddead = 0;
    analyze (p);
  }while (founddead);
  Log_str ("analyzing dead code finished:");
  
  
  Log_str ("rewriting starting:");
  p = rewriteProg (p);  
  Log_str ("rewriting finished:");
  
  Property_clear (usedProp);
  Property_clear (deadProp);
  return p;
}

static void printArg (Ssa_Prog_t p)
{
  File_t file = File_open ("ssa_deadCode.arg", "w+");
  Ssa_Prog_print (file, p);
  File_close (file);  
  return;
}

static void printResult (Ssa_Prog_t p)
{
  File_t file = File_open ("ssa_deadCode.result", "w+");
  Ssa_Prog_print (file, p);
  File_close (file);  
  return;
}

Ssa_Prog_t Ssa_deadCode (Ssa_Prog_t p)
{
  Ssa_Prog_t r;

  Log_POS();

  Trace_TRACE("Ssa_deadCode"
              , Ssa_deadCodeTraced
              , (p)
              , printArg
              , r
              , printResult);
  return r;
}
