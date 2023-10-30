#include "control.h"
#include "verbose.h"
#include "log.h"
#include "pass.h"

Pass_t Pass_new(String_t name, Verbose_t level, Poly_t thunk, Poly_t (*a)(Poly_t)) {
    Pass_t x = {name, level, thunk, a};
    return x;
}

Poly_t Pass_doit(Pass_t *p) {
    Poly_t r;

    // if this pass is to be dropped, then do nothing
    if (Control_mayDropPass(p->name))
        return p->thunk;

    // if this pass is to be logged, then do this
    if (Control_logPass(p->name)) {
        Log_set(p->name);
    }

    Verbose_TRACE (p->name, p->action, (p->thunk), r, p->level);

    // reset the log
    if (Control_logPass(p->name))
        Log_reset();

    return r;
}
