#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../lib/assert.h"
#include "../lib/mem.h"
#include "../lib/list.h"
#include "../lib/string.h"
#include "../lib/file.h"
#include "../lib/mem.h"
#include "../lib/int.h"
#include "../lib/io.h"
#include "../lib/gc.h"
#include "../lib/hash.h"
#include "../control/command-line.h"
#include "../control/pass.h"
#include "../control/version.h"
#include "../main/compile.h"
#include "main-main.h"

static String_t firstFile = 0;

static void showBanner() {
    time_t now;
    String_t timex, banner;

    time(&now);
    timex = ctime(&now);
    timex[strlen(timex) - 1] = '\0';
    banner = String_concat("dragon ", Version_version, " (built ", timex, ")", 0);
    printf("%s\n", banner);
    return;
}

static void deleteFiles(List_t files) {
    List_t p;

    Assert_ASSERT(files);
    p = List_getFirst(files);
    while (p) {
        String_t cmd, f = (String_t) p->data;
        p = p->next;
        cmd = String_concat("rm -f \"", f, "\"", 0);
        system(cmd);
    }
    return;
}

static List_t objFiles = 0;

static int assembleOne(String_t file) {
    int i = 0;
    String_t objFile, cmd1, cmd2;

    objFile = String_concat(Control_asmDirectory, "fileo-", Int_toString(i++), ".o", 0);
    List_insertLast(objFiles, objFile);
    cmd1 = String_concat("gcc -c -g ", "-I \"", Control_headerDirectory, "\" -o \"", objFile, "\"", "  \"", file, "\"",
                         0);
    if (Control_Verb_order
            (VERBOSE_DETAIL, Control_verbose)) {
        Io_printSpaces(6);
        printf("%s\n", cmd1);
    }
    system(cmd1);
    return 0;
}

static List_t assemble(List_t files) {
    List_t tmp;

    objFiles = List_new();
    List_foreach(files, (Poly_tyVoid) assembleOne);
    tmp = objFiles;
    objFiles = 0;
    return tmp;
}

static String_t link(List_t files) {
    String_t exeFile, libFile, cmd;
    List_t p;

    Assert_ASSERT(files);
    exeFile = (Control_o) ?
              String_concat(Control_o, ".out", 0) :
              ("a.out");
    libFile = String_concat(Control_libDirectory, "main.c ../src/runtime/dragon-lib.c", 0);
    cmd = String_concat("gcc -g -o ", exeFile, 0);
    if (Control_Verb_order
            (VERBOSE_DETAIL, Control_verbose)) {
        Io_printSpaces(6);
        printf("%s", cmd);
    }
    p = List_getFirst(files);
    while(p) {
        String_t s = (String_t) p->data;
        if (Control_Verb_order(VERBOSE_DETAIL, Control_verbose)) {
            Io_printSpaces(13);
            printf("%s", s);
        }
        cmd = String_concat(cmd, " ", s, 0);
        p = p->next;
    }
    if (Control_Verb_order(VERBOSE_DETAIL, Control_verbose)) {
        Io_printSpaces(13);
        printf("\"%s\"", libFile);
    }
    cmd = String_concat(cmd, " ", libFile, 0);
    printf("%s\n", cmd);
    system(cmd);
    return exeFile;
}

// Compile a file to assembly file, then assemble to 
// binary file; finally, link the binary with
// libraries to generate final executable.
static int Main_main0(List_t files) {
    Pass_t compile, assemblePass, linkPass;
    List_t asmFiles, objFiles;
    File_t exeFile;

    compile = Pass_new("compile", VERBOSE_PASS, files, (Poly_tyId) Compile_compile);
    asmFiles = Pass_doit(&compile);

    assemblePass = Pass_new("assemble", VERBOSE_PASS, asmFiles, (Poly_tyId) assemble);
    objFiles = Pass_doit(&assemblePass);
    //deleteFiles (asmFiles);

    linkPass = Pass_new("link", VERBOSE_PASS, objFiles, (Poly_tyId) link);
    exeFile = Pass_doit(&linkPass);
    //deleteFiles (objFiles);

    return 0;
}

int Main_main(int argc, char **argv) {
    Pass_t mainp;
    List_t files;

    // We are using the Boehm garbage collect, which
    // requires the initialization of the memory
    // management component.
    // This code does this.
    // Read online info. here:
    //   http://www.hpl.hp.com/personal/Hans_Boehm/gc/
    Mem_init();


    // to test the libraries
    //Lib_test ();

    // If only type "dragon", then print some info and exit.
    if (argc < 2) {
        showBanner();
        return 0;
    }
    // else cook commandline arguments.
    // Return a list of file names.
    // E.g.: for the input
    //   dragon -expert true -verbose 2 a.c
    // will return ["a.c"].
    files = CommandLine_doarg(--argc, ++argv);

    // Change this to only one file???
    if (List_isEmpty(files))
        return 0;

    firstFile = (String_t) List_nth(files, 0);

    mainp = Pass_new("dragon", VERBOSE_PASS, files, (Poly_tyId) Main_main0);
    Pass_doit(&mainp);

    if (Control_Verb_order
            (VERBOSE_DETAIL, Control_verbose)) {
        Mem_status();
        Hash_statusAll();
    }
    return 0;
}


