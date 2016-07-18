#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/trace.h"
#include "x86-codegen.h"

static List_t allStms = 0;

static void emit (X86_Stm_t s)
{
  List_insertLast (allStms, s);
  return;
}

#if 0
static List_t getBeforeClearStms ()
{
  List_t t = allStms;
  allStms = 0;
  return t;
}

typedef enum{
  POS_INT,
  POS_GLOBAL,
  POS_STACK,
  POS_MEM
} Pos_t;

typedef struct
{
  Pos_t pos;
  X86_Operand_t op;
} Operand_Result_t;

/* may occupy register ecx, edx.
 * Invariant: ecx: base, edx: offset
 * final: ecx is occupied.
 */
static X86_Operand_t Trans_operand (Machine_Operand_t o)
{
  Assert_ASSERT(o);
  switch (o->kind){
  case MACHINE_OP_INT:
    return X86_Operand_new_int (o->u.intlit);
  case MACHINE_OP_GLOBAL:
    return X86_Operand_new_global (o->u.global);
  case MACHINE_OP_INSTACK:{
    if (o->u.index>0)
      return X86_Operand_new_inStack (-4*(o->u.index));
    else if (o->u.index<0){
      return X86_Operand_new_inStack (-4*(o->u.index-1));
    }
    Error_impossible ();
    return 0;
  }
  case MACHINE_OP_VAR:
    Error_impossible ();
    return 0;
  case MACHINE_OP_STRUCT:{
    emit (X86_Stm_new_moveri
          (X86_EDX,
           o->u.structt.scale 
           * o->u.structt.index));
    if (o->u.structt.base>0)
      emit (X86_Stm_new_load (X86_ECX,
                              X86_Operand_new_inStack
                              ((-4)*o->u.structt.base)));
    else if (o->u.structt.base<0)
      emit (X86_Stm_new_load (X86_ECX,
                              X86_Operand_new_inStack
                              ((-4)*(o->u.structt.base-1))));
    else Error_impossible ();
      
    emit (X86_Stm_new_bop (X86_ECX,
                           OP_ADD,
                           X86_EDX));
    return X86_Operand_new_mem (X86_ECX, 0);
  }
  case MACHINE_OP_ARRAY:{
    emit (X86_Stm_new_load (X86_EDX,
                            X86_Operand_new_inStack
                            ((-4)*o->u.array.offset)));
    /* this should go to machine */
    emit (X86_Stm_new_inc (X86_EDX));
    emit (X86_Stm_new_load (X86_ECX,
                            X86_Operand_new_int 
                            (o->u.array.scale)));
    emit (X86_Stm_new_bop (X86_EDX,
                           OP_TIMES,
                           X86_ECX));
    if (o->u.structt.base>0)
      emit (X86_Stm_new_load (X86_ECX,
                              X86_Operand_new_inStack
                              ((-4)*o->u.structt.base)));
    else if (o->u.structt.base<0)
      emit (X86_Stm_new_load (X86_ECX,
                              X86_Operand_new_inStack
                              ((-4)*(o->u.structt.base-1))));
    else Error_impossible ();
      
    emit (X86_Stm_new_bop (X86_ECX,
                           OP_ADD,
                           X86_EDX));
    return X86_Operand_new_mem (X86_ECX, 0);
  }
  default:
    printf ("%d\n", o->kind);
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

static void genBop (Machine_Operand_t left, 
                    Operator_t op,
                    Machine_Operand_t right,
                    Machine_Operand_t dest)
{
  switch (op){
  case OP_ADD:
  case OP_SUB:
  case OP_TIMES:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_bop (X86_EAX,
                           op,
                           X86_EDX));
    newDest = Trans_operand (dest);
    emit (X86_Stm_new_store (newDest,
                            X86_EAX));
    return;
  }
  case OP_DIVIDE:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_ECX,
                            newRight));
    emit (X86_Stm_new_cltd ());
    emit (X86_Stm_new_uop (X86_ECX,
                           OP_DIVIDE,
                           X86_ECX));
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_MODUS:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_ECX,
                            newRight));
    emit (X86_Stm_new_cltd ());    
    emit (X86_Stm_new_uop (X86_ECX,
                           OP_DIVIDE,
                           X86_ECX));
    emit (X86_Stm_new_moverr (X86_EAX, X86_EDX));
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_LT:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_setl (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_LE:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand (right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_setle (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_GT:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_setg (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_GE:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_setge (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_EQ:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_sete (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  case OP_NE:{
    X86_Operand_t newLeft;
    X86_Operand_t newRight;
    X86_Operand_t newDest;

    newLeft = Trans_operand (left);    
    emit (X86_Stm_new_load (X86_EAX,
                            newLeft));
    newRight = Trans_operand(right);    
    emit (X86_Stm_new_load (X86_EDX,
                            newRight));
    emit (X86_Stm_new_cmp (X86_EAX,
                           X86_EDX));
    emit (X86_Stm_new_setne (X86_AL));
    emit (X86_Stm_new_extendAl ());
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  default:
    Error_impossible ();
    return;
  }
  Error_impossible ();
  return;
}

static void genUop (Machine_Operand_t src,
                    Operator_t op,
                    Machine_Operand_t dest)
{
  switch (op){
  case OP_NEG:{
    X86_Operand_t newSrc, newDest;      

    newSrc = Trans_operand(src);
    emit (X86_Stm_new_load (X86_EAX,
                            newSrc));
    emit (X86_Stm_new_neg (X86_EAX));
    newDest = Trans_operand(dest);
    emit (X86_Stm_new_store (newDest,
                             X86_EAX));
    return;
  }
  default:
    Error_impossible ();
    return;
  }
  Error_impossible ();
  return;
}

/* Convention for binary operation:
 *   x   bop y
 *   |       |
 *   \/      \/
 *  eax     edx
 * For some operation, this has the advantage of 
 * simplifying the generated instructions.
 */
static void Trans_stm (Machine_Stm_t s)
{
  Assert_ASSERT(s);
  switch (s->kind){
  case MACHINE_STM_MOVE:{
    X86_Stm_t s1, s2;
    X86_Operand_t newSrc, newDest;
    newSrc = Trans_operand (s->u.move.src);
    s1 = X86_Stm_new_load (X86_EAX, newSrc);
    emit (s1);
    newDest = Trans_operand (s->u.move.dest);
    s2 = X86_Stm_new_store (newDest, X86_EAX);
    emit (s2);
    return;
  }
  case MACHINE_STM_BOP:{
    genBop (s->u.bop.left, 
            s->u.bop.op,
            s->u.bop.right,
            s->u.bop.dest);
    return;
  }    
  case MACHINE_STM_UOP:{
    genUop (s->u.uop.src,
            s->u.uop.op,
            s->u.uop.dest);
    return;
  }
  case MACHINE_STM_CALL:{
    int n = 0;
    List_t args = List_getFirst 
      (List_rev (s->u.call.args));
    X86_Operand_t newDest = Trans_operand (s->u.call.dest);
    while (args){
      Machine_Operand_t src = 
        (Machine_Operand_t)args->data;
      X86_Operand_t newSrc = Trans_operand (src);
      emit (X86_Stm_new_load (X86_EAX, newSrc));
      emit (X86_Stm_new_push (X86_EAX));
      n++;
      args = args->next;
    }
    emit (X86_Stm_new_call 
          (Id_fromString 
           (String_concat ("_",
                           Id_toString (s->u.call.name),
                           0))));
    emit (X86_Stm_new_load (X86_EDX, 
                            X86_Operand_new_int (4*n)));
    emit (X86_Stm_new_bop (X86_ESP,
                           OP_ADD,
                           X86_EDX));
    emit (X86_Stm_new_store (newDest, X86_EAX));
    return;
  }
  case MACHINE_STM_IF:{
    X86_Operand_t newSrc = Trans_operand(s->u.iff.src);
    emit (X86_Stm_new_load (X86_EAX, newSrc));
    emit (X86_Stm_new_load (X86_EDX, 
                            X86_Operand_new_int (0)));
    emit (X86_Stm_new_cmp (X86_EAX, X86_EDX));
    emit (X86_Stm_new_je (s->u.iff.falsee));
    emit (X86_Stm_new_jump (s->u.iff.truee));
    return;
  }
  case MACHINE_STM_LABEL:
    emit (X86_Stm_new_label (s->u.label));
    return;
  case MACHINE_STM_JUMP:
    emit (X86_Stm_new_jump (s->u.jump));
    return;
  case MACHINE_STM_RETURN:{
    X86_Operand_t newSrc
      = Trans_operand (s->u.returnn);

    emit (X86_Stm_new_load (X86_EAX, newSrc));
    emit (X86_Stm_new_return ());
    return;
  }
  case MACHINE_STM_NEW_STRUCT:{
    int n = 0;
    X86_Operand_t newDest;
    List_t args = 
      List_getFirst 
      (List_rev(s->u.newStruct.args));
    while (args){
      Machine_Operand_t src = 
        (Machine_Operand_t)args->data;
      X86_Operand_t newSrc = Trans_operand (src);
      emit (X86_Stm_new_load (X86_EAX, newSrc));
      emit (X86_Stm_new_push (X86_EAX));
      n++;
      args = args->next;
    }
    emit (X86_Stm_new_load (X86_EAX, 
                            X86_Operand_new_int (n)));
    emit (X86_Stm_new_push (X86_EAX));
    emit (X86_Stm_new_load (X86_EAX, 
                            X86_Operand_new_global 
                            (s->u.newStruct.type)));
    emit (X86_Stm_new_push (X86_EAX));
    emit (X86_Stm_new_call 
          (Id_fromString ("_Dragon_alloc_struct")));
    emit (X86_Stm_new_load (X86_EDX, 
                            X86_Operand_new_int (8+4*n)));
    emit (X86_Stm_new_bop (X86_ESP,
                           OP_ADD,
                           X86_EDX));
    newDest = Trans_operand (s->u.newStruct.dest);
    emit (X86_Stm_new_store (newDest, X86_EAX));
    return;
  }
  case MACHINE_STM_NEW_ARRAY:{
    X86_Operand_t newDest, newSize, newInit;

    newInit = Trans_operand (s->u.newArray.init);
    emit (X86_Stm_new_load (X86_EAX,
                            newInit));
    emit (X86_Stm_new_push (X86_EAX));
    newSize = Trans_operand (s->u.newArray.size);
    emit (X86_Stm_new_load (X86_EAX,
                            newSize));
    emit (X86_Stm_new_push (X86_EAX));
    emit (X86_Stm_new_load 
          (X86_EAX,                      
           X86_Operand_new_int (s->u.newArray.isPtr)));
    emit (X86_Stm_new_push (X86_EAX));     
    emit (X86_Stm_new_call 
          (Id_fromString ("_Dragon_alloc_array")));
    emit (X86_Stm_new_load (X86_EDX,
                            X86_Operand_new_int (12)));
    emit (X86_Stm_new_bop (X86_ESP,
                           OP_ADD,
                           X86_EDX)); 
    newDest = Trans_operand (s->u.newArray.dest); 
    emit (X86_Stm_new_store (newDest, X86_EAX));
    return;
  }
  default:
    Error_impossible ();
    return;
  }
  Error_impossible ();
  return;
}

static List_t Trans_stms (List_t stms)
{
  List_t newStms;
  Assert_ASSERT(stms);
  allStms = List_new ();
  List_foreach (stms,
                      (Poly_tyVoid)Trans_stm);
  newStms = getBeforeClearStms ();
  return newStms;
}

static X86_Struct_t Trans_struct (Machine_Struct_t str)
{
  Assert_ASSERT(str);
  return X86_Struct_new (str->type,
                         str->var);
}

static X86_Fun_t Trans_func (Machine_Fun_t f)
{
  List_t stms;

  Assert_ASSERT(f);
  stms = Trans_stms (f->stms);
  return X86_Fun_new 
    (f->type,
     f->name,
     List_map (f->args,
                     (Poly_tyId)Trans_struct),
     List_map (f->decs,
                     (Poly_tyId)Trans_struct),
     stms,
     f->retId,
     f->entry,
     f->exitt);
}

static X86_Str_t Trans_str (Machine_Str_t s)
{
  Assert_ASSERT(s);
  return X86_Str_new (s->name,
                      s->value);
}

static X86_Mask_t Trans_mask (Machine_Mask_t m)
{
  Assert_ASSERT(m);
  return X86_Mask_new (m->name,
                       m->size,
                       m->index);
}

#endif // 0

static X86_Prog_t X86_codegenTraced (Machine_Prog_t p)
{
  List_t strs, masks, funcs;

  Assert_ASSERT(p);
  strs = List_new ();
  masks = List_new ();
  funcs = List_new ();
  return X86_Prog_new (strs
                       , masks
                       , funcs);
}

static void outArg (Machine_Prog_t p)
{
  return;
}

static void outResult (X86_Prog_t p)
{
  return;
}


X86_Prog_t X86_codegen (Machine_Prog_t p)
{
  X86_Prog_t r;
  
  Trace_TRACE("x86_codegen"
              , X86_codegenTraced
              , (p)
              , outArg
              , r
              , outResult);
  return r;
}
