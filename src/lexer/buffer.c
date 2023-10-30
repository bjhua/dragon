#include <stdio.h>
#include "../lib/error.h"
#include "../lib/string.h"
#include "../control/error-msg.h"
#include "buffer.h"

// Defining the buffer size to be 4096 has
// nothing special, but this happens to be
// the size of a physical page.
#define BUF_SIZE 4096

struct File_t {
    String_t name;
    FILE *fd;
};

static struct File_t file = {0, 0};

struct Buffer_t {
    char buf[BUF_SIZE];
    int forward;
};

/*
  ---------------------------------
  |--------------------|          |
  ---------------------------------
                        ^           ^
                        |           |
                      forward     BUF_SIZE
  
  Invariant: "forward" always points to the next
  position of the remaining input
*/
static struct Buffer_t buffer;


static void fillBuffer() {
    int r;

    buffer.forward = 0;
    if (feof(file.fd)) {
        buffer.buf[0] = EOF;
        return;
    }
    r = fread(buffer.buf, sizeof(char), BUF_SIZE, file.fd);
    if (r < BUF_SIZE) {
        buffer.buf[r] = EOF;
    }
    return;
}

int Buffer_getChar() {
    if (buffer.forward >= BUF_SIZE) {
        fillBuffer();
    }
    return buffer.buf[buffer.forward++];
}

void Buffer_putChar() {
    buffer.forward--;
    return;
}

void Buffer_init(String_t fname) {
    file.name = fname;
    file.fd = fopen(fname, "r");
    if (file.fd == 0)
        ErrorMsg_die(String_concat("can not open file: ", fname, 0));
    buffer.forward = BUF_SIZE;
    return;
}
