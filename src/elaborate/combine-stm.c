#include "../lib/assert.h"
#include "../lib/list.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include "combine-stm.h"

static Ast_Block_t Elab_block (Ast_Block_t);

static Ast_Stm_t Elab_stm (Ast_Stm_t s)
{
  Assert_ASSERT(s);
  switch (s->kind){
  case AST_STM_EXP:
    return s;
  case AST_STM_IF:{
    return Ast_Stm_new_if (s->u.iff.cond
                           , Elab_stm (s->u.iff.then)
                           , Elab_stm (s->u.iff.elsee)
                           , s->region);
  }
  case AST_STM_WHILE:{
    return Ast_Stm_new_while (s->u.whilee.cond
                              , Elab_stm (s->u.whilee.body)
                              , s->region);
  }
  case AST_STM_DO:{
    return Ast_Stm_new_do (s->u.doo.cond
                           , Elab_stm (s->u.doo.body)
                           , s->region);
  }
  case AST_STM_FOR:{
    return Ast_Stm_new_for (s->u.forr.header
                            , s->u.forr.cond
                            , s->u.forr.tail
                            , Elab_stm(s->u.forr.body)
                            , s->region);
  }
  case AST_STM_BREAK:
    return s;
  case AST_STM_CONTINUE:
    return s;
  case AST_STM_RETURN:{
    return s;
  }
  case AST_STM_THROW:{
    return s;
  }
  case AST_STM_TRYCATCH:{
    return Ast_Stm_new_tryCatch (Elab_stm(s->u.trycatch.tryy)
                                 , Elab_stm(s->u.trycatch.catchh)
                                 , s->region);
  }
  case AST_STM_BLOCK:   
    return Ast_Stm_new_block (Elab_block (s->u.block));
  default:
    Error_impossible ();
    return s;
  }
  Error_impossible ();
  return s;
}

struct Dec_Result_t 
{
  List_t genDecs;
  List_t genStms;
};

static struct Dec_Result_t Elab_checkDecs (List_t decs)
{
  struct Dec_Result_t result;
  List_t p;
  
  Assert_ASSERT(decs);
  
  p = List_getFirst (decs);
  result.genDecs = List_new ();
  result.genStms = List_new ();
  while (p){
    Ast_Dec_t dec = (Ast_Dec_t)p->data;

    List_insertLast (result.genDecs
                     , Ast_Dec_new (dec->type
                                    , dec->var
                                    , 0));
    if (dec->init){
      List_insertLast 
        (result.genStms,
         Ast_Stm_new_exp 
         (Ast_Exp_new_assign 
          (Ast_Exp_new_lval (Ast_Lval_new_var 
                             (dec->var
                              , dec->init->ty
                              , Region_bogus ())
                             , dec->init->ty),
           dec->init
           , dec->init->ty
           , Region_bogus ())));
    }
    p = p->next;
  }
  return result;
}

static Ast_Block_t Elab_block (Ast_Block_t b)
{
  struct Dec_Result_t gens;
  List_t stms;

  Assert_ASSERT(b);
  gens = Elab_checkDecs(b->decs);
  stms = List_map (b->stms, 
                   (Poly_tyId)Elab_stm);
  List_append (gens.genStms, stms);
  return Ast_Block_new (gens.genDecs, gens.genStms);
}

static Ast_Fun_t Elab_funForOne (Ast_Fun_t f)
{
  Ast_Block_t b;

  Assert_ASSERT(f);
  b = Elab_block (f->block);
  return Ast_Fun_new (f->type,
                      f->name,
                      f->args,
                      b,
                      f->region);
}

static List_t Elab_funDecs (List_t fs)
{
  List_t list;
  
  Assert_ASSERT(fs);
  list = List_map
    (fs, 
     (Poly_tyId)Elab_funForOne);
  return list;
}

static Ast_Prog_t Combine_stm_traced (Ast_Prog_t p)
{
  List_t funcs;

  Assert_ASSERT(p);
  funcs = Elab_funDecs (p->funcs);
  return Ast_Prog_new (p->classes, funcs);
}

static void Trace_arg (Ast_Prog_t p)
{
  File_saveToFile ("Combine-stm.arg"
                   , (Poly_tyPrint)Ast_Prog_print
                   , p);
  return;
}

static void Trace_result (Ast_Prog_t p)
{
  File_saveToFile ("Combine-stm.result"
                   , (Poly_tyPrint)Ast_Prog_print
                   , p);
  return;
}

Ast_Prog_t Combine_stm (Ast_Prog_t p)
{
  Ast_Prog_t r;

  Trace_TRACE("Combine_stm"
              , Combine_stm_traced
              , (p)
              , Trace_arg
              , r
              , Trace_result);
  return r;
}
