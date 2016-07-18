#include "../lib/string.h"
#include "../lib/assert.h"
#include "../lib/mem.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"
#include "../lib/poly.h"
#include "../lib/int.h"
#include "control.h"

String_t Control_asmDirectory = 
  "c:\\program files\\dragon\\temp\\";
String_t Control_libDirectory = 
  "c:\\program files\\dragon\\lib\\";
String_t Control_headerDirectory = 
  "c:\\program files\\dragon\\lib\\include";

typedef struct Flag_t *Flag_t;
struct Flag_t
{
  String_t name;
  /* <String_t, String_t> */
  Tuple_t (*toString)();
  void (*reset)();
};

Flag_t Flag_new (String_t name, 
                 Tuple_t (*toString)(), 
                 void (*reset)())
{
  Flag_t f;

  Mem_NEW (f);
  f->name = name;
  f->toString = toString;
  f->reset = reset;
  return f;
}

static struct List_t allFlagsHead = {0, 0};
static List_t allFlags = &allFlagsHead;

#define Flag_add(name, f)                                      \
  List_insertLast(allFlags,                              \
                        Flag_new (name,                        \
                                  f##ToString,                 \
                                  f##Reset));





/* assert */
Tuple_t Control_assertToString()
{
  if (Assert_flag)
    return Tuple_new ("true", "false");
  return Tuple_new ("false", "false");
}

void Control_assertReset ()
{
  Assert_flag = 0;
}

/* buffer size */
int Control_bufferSize = 16;
int Control_bufferSizeDefault = 16;
Tuple_t Control_bufferSizeToString ()
{
  return 
    Tuple_new(Int_toString (Control_bufferSize),
              Int_toString (Control_bufferSizeDefault));
}
void Control_bufferSizeReset ()
{
  Control_bufferSize = Control_bufferSizeDefault;
}

/* code gen */
Codegen_t Control_codegen = CODEGEN_C;
static Codegen_t Control_codegenDefault = CODEGEN_C;
static Tuple_t Control_codegenToString ()
{
  return 
    Tuple_new((Control_codegen==CODEGEN_X86)?
              "x86": "C"
              , "x86");
}
static void Control_codegenReset ()
{
  Control_codegen = Control_codegenDefault;
}

////////////////////////////////////////////////////
/* drop pass */
static struct List_t head = {0, 0};
static List_t Control_dropPass = &head;
static List_t Control_dropPassDefault = 0;
static Tuple_t Control_dropPassToString ()
{
  return Tuple_new (List_toString 
                    (Control_dropPass,
                     ", ",
                     (Poly_tyToString)String_toString),
                    "[]");
}

static void Control_dropPassReset ()
{
  Control_dropPass = Control_dropPassDefault;
}

int Control_mayDropPass (String_t name)
{
  return List_exists (Control_dropPass,
                            name,
                            (Poly_tyEquals)String_equals);
}

void Control_dropPass_insert (String_t name)
{
  List_insertLast (Control_dropPass, name);
}

/////////////////////////////////////////////////////////
/* dump */
static List_t Control_dump = 0;
static List_t Control_dumpDefault = 0;
static String_t Control_dumpFlagToString (Dump_t d)
{
  switch (d){
  case DUMP_AST:
    return "ast";
  case DUMP_HIL:
    return "hil";
  case DUMP_TAC:
    return "tac";
  case DUMP_MACHINE:
    return "machine";
  case DUMP_C:
    return "C";
  case DUMP_X86:
    return "x86";
  default:
    Error_bug ("impossible");
    return "<bogus>";
  }
  Error_impossible ();
  return "<bogus>";
}
String_t Control_dumpToString2 ()
{
  if (!Control_dump)
    return "[]";
  return List_toString
    (Control_dump,
     ", ",
     (Poly_tyToString)Control_dumpFlagToString);
}

Tuple_t Control_dumpToString ()
{
  return Tuple_new (Control_dumpToString2 (), "[]");
}

void Control_dumpReset ()
{
  Control_dump = Control_dumpDefault;
}

void Control_dump_insert (Dump_t il)
{
  if (!Control_dump)
    Control_dump = List_new ();
  List_insertLast (Control_dump, (Poly_t)il);
  return;
}

int Control_dump_lookup (Dump_t il)
{
  List_t p;
  
  if (!Control_dump)
    return 0;
  p = List_getFirst (Control_dump);
  while (p){
    if ((Dump_t)p->data == il)
      return 1;
    p = p->next;
  }
  return 0;
}

///////////////////////////////////////////////////////
// expert
Expert_t Control_expert = EXPERT_NORMAL;
Expert_t Control_expertDefault = EXPERT_NORMAL;
String_t Control_expertToString2 ()
{
  switch (Control_expert){
  case EXPERT_NORMAL:
    return "false";
  case EXPERT_EXPERT:
    return "true";
  default:
    Error_bug ("impossible");
    return "<bogus>";
  }
}

Tuple_t Control_expertToString ()
{
  return Tuple_new (Control_expertToString2 (), "false");
}

void Control_expertReset ()
{
  Control_expert = Control_expertDefault;
}

/////////////////////////////////////////////////////
// label info
int Control_labelInfo = 0;
int Control_labelInfoDefault = 0;
Tuple_t Control_labelInfoToString ()
{
  if (Control_labelInfo)
    return Tuple_new ("true", "false");
  return Tuple_new ("false", "false");
}

void Control_labelInfoReset ()
{
  Control_labelInfo = Control_labelInfoDefault;
}


//////////////////////////////////////////////////////
// log pass
static struct List_t logPassHead = {0, 0};
static List_t Control_logPasses = &logPassHead;
static List_t Control_logPassDefault = 0;
static Tuple_t Control_logPassToString ()
{
  return Tuple_new (List_toString 
                    (Control_logPasses,
                     ", ",
                     (Poly_tyToString)String_toString),
                    "[]");
}

static void Control_logPassReset ()
{
  Control_logPasses = Control_logPassDefault;
}

int Control_logPass (String_t name)
{
  int r;

  r = List_exists (Control_logPasses,
                   name,
                   (Poly_tyEquals)String_equals);
  return r;
}

void Control_logPass_insert (String_t name)
{
  List_insertLast (Control_logPasses, name);
}

/////////////////////////////////////////////////////
/* o */
String_t Control_o = 0;
static String_t Control_oDefault = 0;
static Tuple_t Control_oToString()
{
  return Tuple_new ((Control_o)? Control_o: "\"\"", 
                    "\"\"");
}
static void Control_oReset ()
{
  Control_o = Control_oDefault;
}

/////////////////////////////////////////////////////
// target-size
int Control_Target_size = 4;
static int Control_Target_sizeDefault = 4;
static Tuple_t Control_Target_sizeToString()
{
  return Tuple_new (Int_toString (Control_Target_size)
                    , Int_toString (Control_Target_sizeDefault));
}
static void Control_Target_sizeReset ()
{
  Control_Target_size = Control_Target_sizeDefault;
}

////////////////////////////////////////////////////////
/* show type */
int Control_showType = 0;
static int Control_showTypeDefault = 0;
static Tuple_t Control_showTypeToString()
{
  return Tuple_new (Control_showType? "true": "false",
                    "false");
}
static void Control_showTypeReset ()
{
  Control_showType = Control_showTypeDefault;
}

////////////////////////////////////////////////////////
// keep jpg files
int Control_jpg = 0;
int Control_jpgDefault = 0;
static Tuple_t Control_jpgToString()
{
  return Tuple_new (Control_jpg? "true": "false",
                    "false");
}
static void Control_jpgReset ()
{
  Control_jpg = Control_jpgDefault;
}

////////////////////////////////////////////////////////
/* trace */
List_t Control_trace = 0;
List_t Control_traceDefault = 0;
String_t Control_traceToString2()
{
  return List_toString
    (Trace_allFuncs (), 
     ", ",
     (Poly_tyToString)String_toString);
}
Tuple_t Control_traceToString ()
{
  return Tuple_new (Control_traceToString2(), "[]");
}
void Control_traceReset ()
{
  Trace_reset ();
}



//////////////////////////////////////////////////////
// verbose
Verbose_t Control_verbose = VERBOSE_SILENT;
Verbose_t Control_verboseDefault = VERBOSE_SILENT;
String_t Control_verboseToString2()
{
  switch (Control_verbose){
  case VERBOSE_SILENT:
    return "0";
  case VERBOSE_PASS:
    return "1";
  case VERBOSE_SUBPASS:
    return "2";
  case VERBOSE_DETAIL:
    return "3";
  default:
    Error_bug ("impossible");
    return "<bogus>";
  }
}
Tuple_t Control_verboseToString()
{
  return Tuple_new (Control_verboseToString2(), "0");
}
void Control_verboseReset ()
{
  Control_verbose = Control_verboseDefault;
}

int Control_Verb_order (Verbose_t l1, Verbose_t l2)
{
  return l1<=l2;
}

void Control_init ()
{
  Flag_add("assert flag: ", Control_assert);
  Flag_add("bufferSize flag: ", Control_bufferSize);
  Flag_add("code gen: ", Control_codegen);
  Flag_add("drop pass flag: ", Control_dropPass);
  Flag_add("dump flag: ", Control_dump);
  Flag_add("expert flag: ", Control_expert);
  Flag_add("jpg flag: ", Control_jpg);
  Flag_add("labelInfo flag: ", Control_labelInfo);
  Flag_add("logPass flag: ", Control_logPass);
  Flag_add("output name flag: ", Control_o);
  Flag_add("show type flag: ", Control_showType);
  Flag_add("trace flag: ", Control_trace);
  Flag_add("verbose flag: ", Control_verbose);
  return;
}

void Control_printFlags ()
{
  List_t p = List_getFirst (allFlags);
  Flag_t f;
  Tuple_t tuple;

  printf ("Flag setttings:\n");
  while (p){
    f = (Flag_t)p->data;
    printf ("   %s\n", f->name);
    tuple = f->toString ();
    printf ("      current: %s\n", 
            (String_t)Tuple_first(tuple));
    printf ("      default: %s\n", 
            (String_t)Tuple_second (tuple));
    p = p->next;
  }
  return;
}
