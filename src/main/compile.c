#include "../lib/string.h"
#include "../lib/file.h"
#include "../lib/tuple.h"
#include "../lib/error.h"
#include "../control/command-line.h"
#include "../control/pass.h"
#include "../parser/parse.h"
#include "../ast/ast.h"
#include "../elaborate/elaborate-main.h"
#include "../hil/hil.h"
#include "../hil/hil-main.h"
#include "../ssa/ssa-main.h"
#include "../machine/machine-main.h"
#include "../c-codegen/c-codegen-main.h"
#include "../x86/x86.h"
#include "../x86/x86-main.h"
#include "compile.h"


static String_t Compile_one (String_t file);

static String_t genFileName (String_t f, String_t a)
{
  return String_concat (f, ".", a, 0);
}

static String_t Compile_one (String_t file)
{
  Pass_t lexAndPass, elaborate,
    flatten, ssaPass, machinePass, CPass, x86Pass;
  Ast_Prog_t ast;
  Hil_Prog_t hil;
  Ssa_Prog_t ssa;
  Machine_Prog_t machine;
  Tuple_t tuple;
  X86_Prog_t x86;
  
  lexAndPass = Pass_new ("lexAndParse"
                         , VERBOSE_SUBPASS
                         , file
                         , (Poly_tyId)Parse_parse);
  ast = Pass_doit (&lexAndPass);
  if (Control_dump_lookup (DUMP_AST)){
    File_saveToFile (genFileName ("gen", "ast")
                     , (Poly_tyPrint)Ast_Prog_print
                     , ast);
  }

  elaborate = Pass_new ("elaboration",
                        VERBOSE_SUBPASS,
                        ast,
                        (Poly_tyId)Elaborate_main);
  hil = Pass_doit (&elaborate);
  if (Control_dump_lookup (DUMP_HIL)){
    File_saveToFile (genFileName ("gen", "hil")
                     , (Poly_tyPrint)Hil_Prog_print
                     , hil);
  }
  ast = 0;

  flatten = Pass_new ("hil"
                      , VERBOSE_SUBPASS
                      , hil
                      , (Poly_tyId)Hil_main);
  ssa = Pass_doit (&flatten);

  if (Control_dump_lookup (DUMP_TAC)){
    File_saveToFile (genFileName ("gen", "ssa")
                     , (Poly_tyPrint)Ssa_Prog_print
                     , ssa);
  }
  hil = 0;

  ssaPass = Pass_new ("ssa"
                      , VERBOSE_SUBPASS
                      , ssa
                      , (Poly_tyId)Ssa_main);
  machine = Pass_doit (&ssaPass);
  if (Control_dump_lookup (DUMP_MACHINE)){
    File_saveToFile (genFileName ("gen", "machine")
                     , (Poly_tyPrint)Machine_Prog_print
                     , machine);
  }
  ssa = 0;

  machinePass = Pass_new ("machine"
                          , VERBOSE_SUBPASS
                          , machine
                          , (Poly_tyId)Machine_main);
  machine = Pass_doit (&machinePass);
  /* if (Control_dump_lookup (DUMP_MACHINE)){ */
  /*   File_saveToFile (genFileName (file, "machine"), */
  /*                    Machine_Prog_print, machine); */
  /* } */

  // either we use C codegen or x86 codegen
  switch (Control_codegen){
  case CODEGEN_C:{
    String_t f;

    CPass = Pass_new ("genC"
                      , VERBOSE_SUBPASS
                      , machine
                      , (Poly_tyId)C_codegen_main);
    f = Pass_doit (&CPass);
    machine = 0;
    return f;
  }
  case CODEGEN_X86:{
    x86Pass = Pass_new ("genX86"
                        , VERBOSE_SUBPASS
                        , machine
                        , (Poly_tyId)X86_main);
    tuple = Pass_doit (&x86Pass);
    x86 = Tuple_first (tuple);
    if (Control_dump_lookup (DUMP_X86)){
      File_saveToFile (genFileName (file, "s"),
                       (Poly_tyPrint)X86_Prog_print, x86);
    }
    machine = 0;
    x86 = 0;
    return Tuple_second (tuple);
  }
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

List_t Compile_compile (List_t files)
{
  return List_map (files, (Poly_tyId)Compile_one);
}
