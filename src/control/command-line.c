#include "command-line.h"
#include "../lib/error.h"
#include "../lib/int.h"
#include "../lib/io.h"
#include "../lib/trace.h"
#include "../lib/unused.h"
#include "control.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void errorNoName(String_t);

static void errorWrongArg(String_t, String_t, String_t);

typedef enum {
    ARGTYPE_BOOL,
    ARGTYPE_EMPTY,// expects no argument
    ARGTYPE_INT,
    ARGTYPE_STRING,
} ArgType_t;

//////////////////////////////////////////////////////
/*        all functions */

static void Arg_setBuffer(long i) {
    Control_bufferSize = i;
}

static void Arg_setDropPass(String_t s) {
    Control_dropPass_insert(s);
}

static void Arg_setCodegen(String_t s) {
    if (String_equals(s, "C"))
        Control_codegen = CODEGEN_C;
    else if (String_equals(s, "x86"))
        Control_codegen = CODEGEN_X86;
    else
        errorWrongArg("-codegen", "{C|x86}", s);
    return;
}

static void Arg_setDump(String_t s) {
    if (String_equals(s, "ast"))
        Control_dump_insert(DUMP_AST);
    else if (String_equals(s, "hil"))
        Control_dump_insert(DUMP_HIL);
    else if (String_equals(s, "tac"))
        Control_dump_insert(DUMP_TAC);
    else if (String_equals(s, "machine"))
        Control_dump_insert(DUMP_MACHINE);
    else if (String_equals(s, "C"))
        Control_dump_insert(DUMP_C);
    else if (String_equals(s, "x86"))
        Control_dump_insert(DUMP_X86);
    else
        errorWrongArg("-dump", "<ir>", s);
}

static void Arg_setExpert(long b) {
    Control_expert = b ? EXPERT_EXPERT : EXPERT_NORMAL;
}

static void Arg_setJpg(long b) {
    Control_jpg = b;
}

static void Arg_setLabelInfo(long b) {
    Control_labelInfo = b;
}

static void Arg_setLog(String_t s) {
    Control_logPass_insert(s);
}

static void Arg_setO(String_t s) {
    Control_out_file_name = s;
}

static void Arg_setS(void *arg) {
    UNUSED(arg);

    Control_dump_insert(DUMP_X86);
}

static void Arg_setShowType(void *arg) {
    UNUSED(arg);

    Control_showType = 1;
}

static void Arg_setTrace(String_t s) {
    Trace_insert(s);
}

static void Arg_setVerbose(long i) {
    switch (i) {
        case 0:
            Control_verbose = VERBOSE_SILENT;
            break;
        case 1:
            Control_verbose = VERBOSE_PASS;
            break;
        case 2:
            Control_verbose = VERBOSE_SUBPASS;
            break;
        case 3:
            Control_verbose = VERBOSE_DETAIL;
            break;
        default:
            errorWrongArg("-verbose", "{0|1|2|3}",
                          Int_toString(i));
            break;
    }
}

/* Typically, a commandline argument take the form of:
 *   -name arg        desc

 * for instance:
 *   -o filename      set the output file name

 * the following data structure defines this.
 */
struct Arg_t {
    Expert_t level;             // on what level should this see
    String_t name;              // argument name
    String_t arg;               // argument (for displaying)
    String_t desc;              // argument description
    ArgType_t argtype;          // what type of argument expects
    void (*action)(void *, ...);// a call-back
};
typedef void (*TyArg)(void *, ...);

/* all available arguments */
static struct Arg_t allArgs[] = {
        {EXPERT_NORMAL,
         "buffer",
         "<n>",
         "set output buffer size for file (n M)",
         ARGTYPE_INT,
         (TyArg) Arg_setBuffer},
        {EXPERT_NORMAL, "codegen", "{C|x86}", "which code generator to use", ARGTYPE_STRING, (TyArg) Arg_setCodegen},
        {EXPERT_NORMAL,
         "drop-pass",
         "<pass>",
         "drop this compiler pass",
         ARGTYPE_STRING,
         (TyArg) Arg_setDropPass},
        {EXPERT_NORMAL,
         "dump",
         "<il>",
         "which intermediate "
         "language to dump",
         ARGTYPE_STRING,
         (TyArg) Arg_setDump},
        {EXPERT_EXPERT,
         "expert",
         "{false|true}",
         "show expert level switches",
         ARGTYPE_BOOL,
         (TyArg) Arg_setExpert},
        {EXPERT_NORMAL,
         "jpg",
         "{flase|true}",
         "keep jpg files for ILs (require graphviz)",
         ARGTYPE_BOOL,
         (TyArg) Arg_setJpg},
        {EXPERT_NORMAL,
         "labelinfo",
         "{flase|true}",
         "show extra information on labels",
         ARGTYPE_BOOL,
         (TyArg) Arg_setLabelInfo},
        {EXPERT_EXPERT,
         "log",
         "<pass>",
         "keep logs for a pass",
         ARGTYPE_STRING,
         (TyArg) Arg_setLog},
        {EXPERT_NORMAL,
         "o",
         "<file>",
         "set the output file name",
         ARGTYPE_STRING,
         (TyArg) Arg_setO},
        {EXPERT_NORMAL,
         "S",
         "",
         "keep assembly",
         ARGTYPE_EMPTY,
         (TyArg) Arg_setS},
        {EXPERT_EXPERT,
         "showtype",
         "{false|true}",
         "show type information when dumping ILs",
         ARGTYPE_BOOL,
         (TyArg) Arg_setShowType},
        {EXPERT_EXPERT,
         "trace",
         "<name>",
         "to trace a function",
         ARGTYPE_STRING,
         (TyArg) Arg_setTrace},
        {EXPERT_NORMAL,
         "verbose",
         "{0|1|2|3}",
         "how verbose to be",
         ARGTYPE_INT,
         (TyArg) Arg_setVerbose},
        {EXPERT_NORMAL,
         0,
         0,
         0,
         ARGTYPE_EMPTY,
         0}};


#define LEFT_SIZE 28
#define INDENT_SIZE 3

static void Arg_print(void) {
    for (long i = 0; allArgs[i].action; ++i) {
        if (!Control_Verb_order((Verbose_t) allArgs[i].level, (Verbose_t) Control_expert))
            return;
        long left = INDENT_SIZE + 1 + String_size(allArgs[i].name) + 1 + String_size(allArgs[i].arg) + 1;
        Io_printSpaces(INDENT_SIZE);
        printf("-%s", allArgs[i].name);
        printf(" ");
        printf("%s", allArgs[i].arg);
        if (left > LEFT_SIZE) {
            printf("\n");
            Io_printSpaces(LEFT_SIZE);
        } else
            Io_printSpaces(LEFT_SIZE - left);
        printf(" %s\n", allArgs[i].desc);
    }
    return;
}

static void errorNoName(String_t s) {
    printf("unknown switch: %s\n", s);
    Arg_print();
    exit(0);
}

static void errorNoArg(String_t name,
                       String_t arg) {
    printf("no argument is given to switch: %s\n"
           "expects arg: %s\n",
           name, arg);
    Arg_print();
    exit(0);
}

static void errorWrongArg(String_t name,
                          String_t arg,
                          String_t input) {
    printf("invalid arg for switch: %s\n"
           "expects: %s\n"
           "but got: %s\n",
           name, arg, input);
    Arg_print();
    exit(0);
}

// if some input file is specified, then return it; else
// return 0.
List_t CommandLine_doarg(int argc, char **argv) {
    int index;
    char *inputName;
    List_t files = List_new();

    // TODO: this should also be cleaned!
    Control_init();

    // scan all input command-line arguments
    for (index = 0; index < argc;) {
        inputName = argv[index++];
        // If a string does not start with '-', then
        // treat it as a file name.
        if ('-' != inputName[0]) {
            List_insertLast(files, inputName);
            continue;
        }

        // this is a potential argument
        int i = 0;
        for (; allArgs[i].action; i++) {
            if (!String_equals(inputName + 1, allArgs[i].name)) {
                /* printf ("get: %s\n" */
                /*         "see: %s, continue...\n", inputName+1, */
                /*         allArgs[i].name); */
                continue;
            }

            switch (allArgs[i].argtype) {
                case ARGTYPE_BOOL: {
                    long b = 0;
                    char *arg = 0;

                    if (index >= argc)
                        errorNoArg(allArgs[i].name,
                                   allArgs[i].arg);

                    arg = argv[index++];
                    if (String_equals(arg, "true"))
                        b = 1;
                    else if (String_equals(arg, "false"))
                        b = 0;
                    else
                        errorWrongArg(allArgs[i].name,
                                      allArgs[i].arg,
                                      arg);
                    allArgs[i].action((void *) b);
                    break;
                }
                case ARGTYPE_INT: {
                    long result, erc;
                    char *arg;

                    if (index >= argc)
                        errorNoArg(allArgs[i].name,
                                   allArgs[i].arg);

                    arg = argv[index++];
                    erc = Int_fromString(arg, &result);

                    if (!erc)
                        errorWrongArg(allArgs[i].name,
                                      allArgs[i].arg,
                                      arg);
                    allArgs[i].action((void *) result);
                    break;
                }
                case ARGTYPE_STRING: {
                    char *arg;

                    if (index >= argc)
                        errorNoArg(allArgs[i].name,
                                   allArgs[i].arg);

                    arg = argv[index++];
                    allArgs[i].action(arg);
                    break;
                }
                case ARGTYPE_EMPTY: {
                    allArgs[i].action("");
                    break;
                }
                default: {
                    Error_impossible();
                    break;
                }
            }
            break;
        }
        if (!allArgs[i].action)
            errorNoName(inputName);
    }
    if (Control_Verb_order(VERBOSE_SUBPASS, Control_verbose))
        Control_printFlags();
    return files;
}
