#include "main-main.h"
#include "../control/command-line.h"
#include "../control/pass.h"
#include "../control/version.h"
#include "../lib/hash.h"
#include "../lib/int.h"
#include "../lib/io.h"
#include "../lib/mem.h"
#include "../main/compile.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//static String_t firstFile = 0;

static void show_banner() {
    time_t now;
    String_t timex, banner;

    time(&now);
    timex = ctime(&now);
    timex[strlen(timex) - 1] = '\0';
    banner = String_concat("dragon ", Version_version, " (built ", timex, ")", 0);
    printf("%s\n", banner);
    //return;
}

static String_t assemble_one(String_t file) {
    static long i = 0;
    String_t obj_file = String_concat("/tmp/", "file-o-", Int_toString(i++), ".o", 0);
    String_t cmd = String_concat("gcc -c -g ", "-I \"", Control_headerDirectory, "\" -o \"", obj_file, "\"", "  \"", file, "\"",
                                 0);
    if (Control_Verb_order(VERBOSE_DETAIL, Control_verbose)) {
        Io_printSpaces(6);
        printf("%s\n", cmd);
    }
    system(cmd);
    return obj_file;
}

static List_t assemble(List_t files) {
    List_t obj_files = List_new();
    List_t first = List_getFirst(files);
    while (first) {
        String_t obj_file = assemble_one(first->data);
        List_insertLast(obj_files, obj_file);
        first = first->next;
    }
    return obj_files;
}

static String_t link(List_t files) {
    List_t p;

    assert(files);
    String_t exe_file_name = Control_out_file_name ? Control_out_file_name : "a.out";
    String_t lib_files = String_concat(Control_libDirectory,
                                       "../src/runtime/main.c ../src/runtime/dragon-lib.c",
                                       0);
    String_t cmd = String_concat("gcc -g -o ", exe_file_name, 0);
    if (Control_Verb_order(VERBOSE_DETAIL, Control_verbose)) {
        Io_printSpaces(6);
        printf("%s", cmd);
    }
    p = List_getFirst(files);
    while (p) {
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
        printf("\"%s\"", lib_files);
    }
    cmd = String_concat(cmd, " ", lib_files, 0);
    printf("%s\n", cmd);
    system(cmd);
    return exe_file_name;
}

// Compile a file to assembly file, then assemble to
// binary file; finally, link the binary with
// libraries to generate final executable.
static int Main_main0(List_t files) {
    Pass_t compile, assemblePass, linkPass;

    compile = Pass_new("compile", VERBOSE_PASS, files, (Poly_tyId) Compile_compile);
    List_t asm_files = Pass_doit(&compile);

    assemblePass = Pass_new("assemble", VERBOSE_PASS, asm_files, (Poly_tyId) assemble);
    List_t obj_files = Pass_doit(&assemblePass);
    //deleteFiles (asmFiles);

    linkPass = Pass_new("link", VERBOSE_PASS, obj_files, (Poly_tyId) link);
    Pass_doit(&linkPass);
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
        show_banner();
        return 1;
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

    //firstFile = (String_t) List_nth(files, 0);

    mainp = Pass_new("dragon", VERBOSE_PASS, files, (Poly_tyId) Main_main0);
    Pass_doit(&mainp);

    if (Control_Verb_order(VERBOSE_DETAIL,
                           Control_verbose)) {
        Mem_status();
        Hash_statusAll();
    }
    return 0;
}
