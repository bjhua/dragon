#include "mem.h"
#include "char.h"

/* editor-dependent. */
#define TAB_SIZE 8

/* a subset of the ASCII. */
int Char_isBlank(int c) {
    return c == ' ' || c == '\t';
}

int Char_blankSize(int c) {
    switch (c) {
        case ' ':
            return 1;
        case '\t':
            return TAB_SIZE;
        default:
            return -1;
    }
}

int Char_isDigit(int c) {
    return '0' <= c && c <= '9';
}

int Char_isAlpha(int c) {
    return (('a' <= c && c <= 'z')
            || ('A' <= c && c <= 'Z'));
}

#define BUF_SIZE 1024

String_t Char_toString(int i) {
    char *temp;
    Mem_NEW_SIZE(temp, BUF_SIZE);
    /* Note that I initially want to write:
     *   snprintf (temp, BUF_SIZE, "%d", i);
     * but it seems that this version of GCC I'm using has a bug on
     * the function "snprintf". So I've to write this unsafe "sprintf".
     */
    sprintf (temp, "%c", i);
    return temp;
}

