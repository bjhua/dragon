#include "../lib/assert.h"
#include "../lib/list.h"
#include "../lib/property.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include "trans-ssa.h"

// 
/* List <Machine_Str_t> */
static List_t strings = 0;

static Id_t genStr (String_t s)
{
  Id_t id = Id_newNoName ();
  Machine_Str_t str = Machine_Str_new (id, s);
  List_insertLast (strings, str);
  return id;
}

static List_t getStrings ()
{
  List_t t = strings;
  strings = 0;
  return t;
}


/////////////////////////////////////////////////////
// operands
static Machine_Operand_t Trans_operand (Ssa_Operand_t o)
{
  Assert_ASSERT(o);
  switch (o->kind){
  case SSA_OP_INT:
    return Machine_Operand_new_int (o->u.intlit);
  case SSA_OP_STR:{
    Id_t id = genStr (o->u.strlit);
    return Machine_Operand_new_global (id);
  }
  case SSA_OP_ID:
    return Machine_Operand_new_id (o->u.id);
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

/////////////////////////////////////////////////////
// memory
static Machine_Mem_t Trans_mem (Ssa_Mem_t m)
{
  Assert_ASSERT(m);
  switch (m->kind){
  case SSA_MEM_ARRAY:
    return Machine_Mem_new_array
      (m->u.array.name
       , Trans_operand (m->u.array.index));
  case SSA_MEM_CLASS:
    return Machine_Mem_new_class
      (m->u.class.name
       , m->u.class.field
       , -1);
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

/////////////////////////////////////////////////////
// statement
static Machine_Stm_t Trans_stmEach (Ssa_Stm_t s)
{
  Assert_ASSERT(s);
  switch (s->kind){
  case SSA_STM_MOVE:
    return 
      Machine_Stm_new_move 
      (s->u.move.dest
       , Trans_operand (s->u.move.src));
  case SSA_STM_BOP:
    return Machine_Stm_new_bop
      (s->u.bop.dest
       , Trans_operand (s->u.bop.left)
       , s->u.bop.op
       , Trans_operand (s->u.bop.right));
  case SSA_STM_UOP:
    return Machine_Stm_new_uop
      (s->u.uop.dest
       , s->u.uop.op
       , Trans_operand (s->u.uop.src));
    /*
  case SSA_STM_CALL:
    if (s->u.call.dest)
      return Machine_Stm_new_call
        (s->u.call.dest
         , s->u.call.name
         , List_map (s->u.call.args
                     , (Poly_tyId)Trans_operand)
         , s->u.call.inTry);
    return Machine_Stm_new_callnoassign
        (s->u.call.name
         , List_map (s->u.call.args
                     , (Poly_tyId)Trans_operand)
         , s->u.call.inTry);
    */
  case SSA_STM_STORE:
    return Machine_Stm_new_store 
      (Trans_mem (s->u.store.m)
       , Trans_operand(s->u.store.src));
  case SSA_STM_LOAD:
    return Machine_Stm_new_load
      (s->u.load.dest
       , Trans_mem (s->u.load.m));
  case SSA_STM_NEW_CLASS:
    return Machine_Stm_new_newClass
      (s->u.newClass.dest
       , s->u.newClass.cname);
  case SSA_STM_NEW_ARRAY:
    return Machine_Stm_new_newArray 
      (s->u.newArray.dest
       , s->u.newArray.ty
       , Trans_operand (s->u.newArray.size));
  case SSA_STM_PHI:
    Error_impossible ();
    return 0;
  case SSA_STM_TRY:
    return Machine_Stm_new_try (s->u.try);
  case SSA_STM_TRY_END:
    return Machine_Stm_new_try_end(s->u.tryEnd);
  default:
    fprintf (stderr, "%d", s->kind);
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

/////////////////////////////////////////////////////
// transfer
static Machine_Transfer_t Trans_transfer (Ssa_Transfer_t t)
{
  switch (t->kind){
  case SSA_TRANS_IF:
    return Machine_Transfer_new_if 
      (Trans_operand (t->u.iff.cond)
       , t->u.iff.truee
       , t->u.iff.falsee);
  case SSA_TRANS_JUMP:
    return Machine_Transfer_new_jump (t->u.jump);
  case SSA_TRANS_RETURN:
    return Machine_Transfer_new_return 
      (Trans_operand (t->u.ret));
  case SSA_TRANS_THROW:
    return Machine_Transfer_new_throw ();
  case SSA_TRANS_CALL: // what about "leave" and "normal"?
    if (t->u.call.dest)
      return Machine_Transfer_new_call
        (t->u.call.dest
         , t->u.call.name
         , List_map (t->u.call.args
                     , (Poly_tyId)Trans_operand)
         , t->u.call.leave
         , t->u.call.normal);
    return Machine_Transfer_new_callnoassign
        (t->u.call.name
         , List_map (t->u.call.args
                     , (Poly_tyId)Trans_operand)
         , t->u.call.leave
         , t->u.call.normal);
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

/////////////////////////////////////////////////////
// blocks
static Machine_Block_t Trans_blockEach (Ssa_Block_t b)
{
  List_t newStms;
  Machine_Transfer_t newTransfer;

  newStms = List_map (b->stms
                      , (Poly_tyId)Trans_stmEach);

  newTransfer = Trans_transfer (b->transfer);

  return Machine_Block_new (b->label
                            , newStms
                            , newTransfer);
}


/////////////////////////////////////////////////////
// functions
static Machine_Fun_t Trans_funcEach (Ssa_Fun_t f)
{
  List_t newBlocks;

  Assert_ASSERT(f);
  newBlocks = List_map (f->blocks,
                        (Poly_tyId)Trans_blockEach);
  
  return Machine_Fun_new (f->type
                          , f->name
                          , f->args
                          , f->decs
                          , newBlocks
                          , f->retId
                          , f->entry
                          , f->exitt
                          , -1);
                          
}

//////////////////////////////////////////////////////
// program
static Machine_Prog_t Trans_ssaTraced (Ssa_Prog_t p)
{
  List_t funcs;
  
  Assert_ASSERT(p);
  
  // clear string caches
  strings = List_new ();

  funcs = List_map (p->funcs, (Poly_tyId)Trans_funcEach);
  
  return Machine_Prog_new (getStrings ()
                           , List_new ()
                           , List_new ()
                           , p->classes
                           , funcs);
}

static void outArg (Ssa_Prog_t p)
{
  Ssa_Prog_toDot (p, "transSsa");
  File_saveToFile ("trans_ssa.arg"
                   , (Poly_tyPrint)Ssa_Prog_print
                   , p);
  return;
}

static void outResult (Machine_Prog_t p)
{
  File_saveToFile ("trans_ssa.result"
                   , (Poly_tyPrint)Machine_Prog_print
                   , p);
  return;
}

Machine_Prog_t Trans_ssa (Ssa_Prog_t p)
{
  Machine_Prog_t r;

  Trace_TRACE("Trans_ssa"
              , Trans_ssaTraced
              , (p)
              , outArg
              , r
              , outResult);
  return r;
}

