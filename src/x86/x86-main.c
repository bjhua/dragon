#include "../lib/int.h"
#include "../control/pass.h"
#include "../control/control.h"
#include "x86.h"
#include "x86-codegen.h"
#include "peep-hole.h"
#include "x86-main.h"

static int counter = 0;

static String_t genFileName ()
{
  String_t f = String_concat 
    ("",
     Control_asmDirectory,
     "files-",
     Int_toString (counter++),
     ".s",
     0);
  return f;
}

static String_t outputX86 (X86_Prog_t p)
{
  String_t f;
  
  f = genFileName ();
  File_saveToFile (f
                   , (Poly_tyPrint)X86_Prog_print
                   , p);
  return f;
}

Tuple_t X86_main (Machine_Prog_t p)
{
  Pass_t codeGen, 
    peepHole,
    output;
  X86_Prog_t p1, p2;
  String_t f;
  
  codeGen = Pass_new ("codegen",
                      VERBOSE_SUBPASS,
                      p,
                      (Poly_tyId)X86_codegen);
  p1 = Pass_doit (&codeGen);

  peepHole = Pass_new ("peephole",
                       VERBOSE_SUBPASS,
                       p1,
                       (Poly_tyId)PeepHole_shrink);
  p2 = Pass_doit (&peepHole);
  p1 = 0;
  
  output = Pass_new ("output",
                     VERBOSE_SUBPASS,
                     p2,
                     (Poly_tyId)outputX86);
  f = Pass_doit (&output);  
  
  return Tuple_new (p2, f);
}
