#ifndef CHAR_H
#define CHAR_H

#include "string.h"

/* return the size of a "blank", in the number of ' ''s. 
 * return 0 if c is not a "blank".
 */
int Char_blankSize (int c);
int Char_isAlpha (int c);
int Char_isBlank (int c);
int Char_isDigit (int c);
String_t Char_toString (int c);

#endif
