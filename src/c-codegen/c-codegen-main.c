#include "../lib/int.h"
#include "../control/pass.h"
#include "c-codegen.h"
#include "c-codegen-main.h"

static int counter = 0;

static String_t genFileName() {
    String_t f;

    if (Control_dump_lookup(DUMP_C)) {
        f = String_concat("files-", Int_toString(counter++), ".c", 0);
        return f;
    }

    f = String_concat("", Control_asmDirectory, "files-", Int_toString(counter++), ".c", 0);
    return f;
}

static String_t out(Machine_Prog_t p) {
    String_t f;
    File_t file;

    f = genFileName();
    file = File_open(f, "w+");
    C_codegen(file, p);
    File_close(file);
    return f;
}

String_t C_codegen_main(Machine_Prog_t p) {
    Pass_t output;
    String_t f;

    output = Pass_new("outputC", VERBOSE_SUBPASS, p, (Poly_tyId) out);
    f = Pass_doit(&output);
    return f;
}
