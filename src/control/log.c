#include "log.h"
#include "../lib/error.h"
#include <assert.h>
#include <stdarg.h>

struct Log_t {
    int effective;// whether this logger is on
    String_t name;// name of the log file
    File_t file;  // log file pointer
};

static struct Log_t log = {0, 0, 0};

void Log_set(String_t name) {
    if (log.effective)
        Error_error("don't support nested log\n");

    log.effective = 1;
    log.name = String_concat(name, ".log", 0);
    log.file = File_open(log.name, "w+");
}

void Log_reset(void) {
    if (!log.effective)
        Error_impossible();

    log.effective = 0;
    log.name = 0;
    File_close(log.file);
}

void Log_dot(void (*f)(Poly_t, String_t), Poly_t p, String_t s) {
    assert(s);
    if (!log.effective)
        return;

    f(p, s);
}

void Log_str(String_t s) {
    assert(s);
    if (!log.effective)
        return;

    File_write(log.file, s);
    File_write(log.file, "\n");
    //
    File_flush(log.file);
}

void Log_strs(String_t s, ...) {
    va_list ap;
    String_t current;

    if (!log.effective)
        return;

    File_write(log.file, s);
    va_start(ap, s);
    current = va_arg(ap, String_t);
    while (current) {
        File_write(log.file, current);
        current = va_arg(ap, String_t);
    }
    //File_write (log.file, "\n");
    va_end(ap);
    File_flush(log.file);
}

void Log_fun(Poly_t a, File_t (*f)(File_t, Poly_t)) {
    if (!log.effective)
        return;

    f(log.file, a);
    File_flush(log.file);
}
