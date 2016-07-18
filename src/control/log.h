#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "../lib/poly.h"
#include "../lib/string.h"
#include "../lib/int.h"
#include "../lib/file.h"

typedef File_t (*Poly_tyLog)(File_t, Poly_t);
typedef void (*Poly_tyDot)(Poly_t, String_t);

#define Log_POS()                                \
  do{                                            \
    Log_strs (__FILE__                           \
              , ": "                             \
              , Int_toString(__LINE__)           \
              , "\n\n"                           \
              , 0);                              \
  }while(0)                                             

// call f on a and str
void Log_dot (Poly_tyDot f
	      , Poly_t a
              , String_t name);
// call the function f on a
void Log_fun (Poly_t a, File_t (*f)(File_t, Poly_t));
// flush current log info, and keep log for nothing now on.
void Log_reset ();
// given the name of pass, keep the log for that pass
void Log_set (String_t passname);
// log a single string
void Log_str (String_t str);
// maybe we should not need such a detailed information.
#define Log_STR(str)                            \
  do{                                           \
    Log_POS();                                  \
    Log_str (str);                              \
  }while(0);
// log a sequence of strings
void Log_strs (String_t str, ...);

#endif
