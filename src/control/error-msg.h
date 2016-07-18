#ifndef ERROR_MSG_H
#define ERROR_MSG_H

#include "../lib/string.h"
#include "region.h"

void ErrorMsg_init ();
void ErrorMsg_errorExit ();
void ErrorMsg_warn (String_t msg);
void ErrorMsg_die (String_t msg);
void ErrorMsg_lexError (char *msg, 
                        char *fname, 
                        int line, 
                        int column);
void ErrorMsg_lexError2 (String_t msg,
                         Coordinate_t t);
void ErrorMsg_syntaxError (char *msg, Region_t r);
void ErrorMsg_elabError (char *msg, Region_t r);

#endif
