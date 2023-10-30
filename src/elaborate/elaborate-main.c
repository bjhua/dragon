#include "../control/pass.h"
#include "check-ast.h"
#include "elaborate.h"
#include "combine-stm.h"
#include "lift-dec.h"
#include "trans-ast.h"
#include "../hil/hil.h"
#include "elaborate-main.h"

Hil_Prog_t Elaborate_main(Ast_Prog_t p) {
    Pass_t elab, check, combine, lift, trans;
    Ast_Prog_t p1, p2, p3;
    Hil_Prog_t q;

    check = Pass_new("well-formedness checking",
                     VERBOSE_SUBPASS,
                     p,
                     (Poly_tyId) Check_ast);
    Pass_doit(&check);

    elab = Pass_new("type checking",
                    VERBOSE_SUBPASS,
                    p,
                    (Poly_tyId) Elaborate_ast);
    p1 = Pass_doit(&elab);

    combine = Pass_new("combine statement",
                       VERBOSE_SUBPASS,
                       p1,
                       (Poly_tyId) Combine_stm);
    p2 = Pass_doit(&combine);

    lift = Pass_new("declaration lifting",
                    VERBOSE_SUBPASS,
                    p2,
                    (Poly_tyId) Lift_dec);
    p3 = Pass_doit(&lift);

    trans = Pass_new("conversion",
                     VERBOSE_SUBPASS,
                     p3,
                     (Poly_tyId) Trans_ast);
    q = Pass_doit(&trans);

    return q;
}
