#include "control.h"
#include "../lib/int.h"
#include "../lib/mem.h"
#include "../lib/poly.h"
#include "../lib/string.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"

String_t Control_asmDirectory = "./";
String_t Control_libDirectory = "./";
String_t Control_headerDirectory = "./";

typedef struct Flag_t *Flag_t;

struct Flag_t {
    String_t name;

    /* <String_t, String_t> */
    Tuple_t (*toString)(void);

    void (*reset)(void);
};

static Flag_t Flag_new(String_t name,
                       Tuple_t (*toString)(void),
                       void (*reset)(void)) {
    Flag_t f;

    Mem_NEW(f);
    f->name = name;
    f->toString = toString;
    f->reset = reset;
    return f;
}

static struct List_t allFlagsHead = {0, 0};
static List_t allFlags = &allFlagsHead;

#define Flag_add(name, f)                 \
    List_insertLast(allFlags,             \
                    Flag_new(name,        \
                             f##ToString, \
                             f##Reset));

/* buffer size */
long Control_bufferSize = 16;
long Control_bufferSizeDefault = 16;

static Tuple_t Control_bufferSizeToString(void) {
    return Tuple_new(Int_toString(Control_bufferSize),
                     Int_toString(Control_bufferSizeDefault));
}

static void Control_bufferSizeReset(void) {
    Control_bufferSize = Control_bufferSizeDefault;
}

/* code gen */
Codegen_t Control_codegen = CODEGEN_C;
static Codegen_t Control_codegenDefault = CODEGEN_C;

static Tuple_t Control_codegenToString(void) {
    return Tuple_new((Control_codegen == CODEGEN_X86) ? "x86" : "C", "x86");
}

static void Control_codegenReset(void) {
    Control_codegen = Control_codegenDefault;
}

////////////////////////////////////////////////////
/* drop pass */
static struct List_t head = {0, 0};
static List_t Control_dropPass = &head;
static List_t Control_dropPassDefault = 0;

static Tuple_t Control_dropPassToString(void) {
    return Tuple_new(List_toString(Control_dropPass,
                                   ", ",
                                   (Poly_tyToString) String_toString),
                     "[]");
}

static void Control_dropPassReset(void) {
    Control_dropPass = Control_dropPassDefault;
}

int Control_mayDropPass(String_t name) {
    return List_exists(Control_dropPass,
                       name,
                       (Poly_tyEquals) String_equals);
}

void Control_dropPass_insert(String_t name) {
    List_insertLast(Control_dropPass, name);
}

/////////////////////////////////////////////////////////
/* dump */
static List_t Control_dump = 0;
static List_t Control_dumpDefault = 0;

static String_t Control_dumpFlagToString(Dump_t d) {
    switch (d) {
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
            Error_bug("impossible");
            return "<bogus>";
    }
    Error_impossible();
    return "<bogus>";
}

static String_t Control_dumpToString2(void) {
    if (!Control_dump)
        return "[]";
    return List_toString(Control_dump,
                         ", ",
                         (Poly_tyToString) Control_dumpFlagToString);
}

static Tuple_t Control_dumpToString(void) {
    return Tuple_new(Control_dumpToString2(), "[]");
}

static void Control_dumpReset(void) {
    Control_dump = Control_dumpDefault;
}

void Control_dump_insert(Dump_t il) {
    if (!Control_dump)
        Control_dump = List_new();
    List_insertLast(Control_dump, (Poly_t) il);
    return;
}

int Control_dump_lookup(Dump_t il) {
    List_t p;

    if (!Control_dump)
        return 0;
    p = List_getFirst(Control_dump);
    while (p) {
        if ((long) p->data == il)
            return 1;
        p = p->next;
    }
    return 0;
}

///////////////////////////////////////////////////////
// expert
Expert_t Control_expert = EXPERT_NORMAL;
Expert_t Control_expertDefault = EXPERT_NORMAL;

static String_t Control_expertToString2(void) {
    switch (Control_expert) {
        case EXPERT_NORMAL:
            return "false";
        case EXPERT_EXPERT:
            return "true";
        default:
            Error_bug("impossible");
            return "<bogus>";
    }
}

static Tuple_t Control_expertToString(void) {
    return Tuple_new(Control_expertToString2(), "false");
}

static void Control_expertReset(void) {
    Control_expert = Control_expertDefault;
}

/////////////////////////////////////////////////////
// label info
long Control_labelInfo = 0;
long Control_labelInfoDefault = 0;

static Tuple_t Control_labelInfoToString(void) {
    if (Control_labelInfo)
        return Tuple_new("true", "false");
    return Tuple_new("false", "false");
}

static void Control_labelInfoReset(void) {
    Control_labelInfo = Control_labelInfoDefault;
}


//////////////////////////////////////////////////////
// log pass
static struct List_t logPassHead = {0, 0};
static List_t Control_logPasses = &logPassHead;
static List_t Control_logPassDefault = 0;

static Tuple_t Control_logPassToString(void) {
    return Tuple_new(List_toString(Control_logPasses,
                                   ", ",
                                   (Poly_tyToString) String_toString),
                     "[]");
}

static void Control_logPassReset(void) {
    Control_logPasses = Control_logPassDefault;
}

int Control_logPass(String_t name) {
    int r;

    r = List_exists(Control_logPasses,
                    name,
                    (Poly_tyEquals) String_equals);
    return r;
}

void Control_logPass_insert(String_t name) {
    List_insertLast(Control_logPasses, name);
}

/////////////////////////////////////////////////////
/* o */
String_t Control_out_file_name = 0;
static String_t Control_oDefault = 0;

static Tuple_t Control_oToString(void) {
    return Tuple_new((Control_out_file_name) ? Control_out_file_name : "\"\"",
                     "\"\"");
}

static void Control_oReset(void) {
    Control_out_file_name = Control_oDefault;
}

/////////////////////////////////////////////////////
// target-size
int Control_Target_size = 4;
//static int Control_Target_sizeDefault = 4;
//
//static Tuple_t Control_Target_sizeToString() {
//    return Tuple_new(Int_toString(Control_Target_size), Int_toString(Control_Target_sizeDefault));
//}

//static void Control_Target_sizeReset() {
//    Control_Target_size = Control_Target_sizeDefault;
//}

////////////////////////////////////////////////////////
/* show type */
long Control_showType = 0;
static int Control_showTypeDefault = 0;

static Tuple_t Control_showTypeToString(void) {
    return Tuple_new(Control_showType ? "true" : "false",
                     "false");
}

static void Control_showTypeReset(void) {
    Control_showType = Control_showTypeDefault;
}

////////////////////////////////////////////////////////
// keep jpg files
long Control_jpg = 0;
long Control_jpgDefault = 0;

static Tuple_t Control_jpgToString(void) {
    return Tuple_new(Control_jpg ? "true" : "false",
                     "false");
}

static void Control_jpgReset(void) {
    Control_jpg = Control_jpgDefault;
}

////////////////////////////////////////////////////////
/* trace */
List_t Control_trace = 0;
List_t Control_traceDefault = 0;

static String_t Control_traceToString2(void) {
    return List_toString(Trace_allFuncs(),
                         ", ",
                         (Poly_tyToString) String_toString);
}

static Tuple_t Control_traceToString(void) {
    return Tuple_new(Control_traceToString2(), "[]");
}

static void Control_traceReset(void) {
    Trace_reset();
}


//////////////////////////////////////////////////////
// verbose
Verbose_t Control_verbose = VERBOSE_SILENT;
Verbose_t Control_verboseDefault = VERBOSE_SILENT;

static String_t Control_verboseToString2(void) {
    switch (Control_verbose) {
        case VERBOSE_SILENT:
            return "0";
        case VERBOSE_PASS:
            return "1";
        case VERBOSE_SUBPASS:
            return "2";
        case VERBOSE_DETAIL:
            return "3";
        default:
            Error_bug("impossible");
            return "<bogus>";
    }
}

static Tuple_t Control_verboseToString(void) {
    return Tuple_new(Control_verboseToString2(), "0");
}

static void Control_verboseReset(void) {
    Control_verbose = Control_verboseDefault;
}

int Control_Verb_order(Verbose_t l1, Verbose_t l2) {
    return l1 <= l2;
}

void Control_init(void) {
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

void Control_printFlags(void) {
    List_t p = List_getFirst(allFlags);
    Flag_t f;
    Tuple_t tuple;

    printf("Flag setttings:\n");
    while (p) {
        f = (Flag_t) p->data;
        printf("   %s\n", f->name);
        tuple = f->toString();
        printf("      current: %s\n",
               (String_t) Tuple_first(tuple));
        printf("      default: %s\n",
               (String_t) Tuple_second(tuple));
        p = p->next;
    }
    //    return;
}
