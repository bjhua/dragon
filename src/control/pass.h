#ifndef PASS_H
#define PASS_H

#include "../lib/string.h"
#include "../lib/poly.h"
#include "control.h"

typedef struct
{
  String_t name;      // name of a compilation pass
  Verbose_t level;    // at what level to see
  Poly_t thunk;       // argument to this pass
  Poly_t (*action)(Poly_t);  // actions. 
} Pass_t;

Pass_t Pass_new (String_t, Verbose_t, Poly_t, Poly_t (*)(Poly_t));
Poly_t Pass_doit (Pass_t *pass);

#endif
