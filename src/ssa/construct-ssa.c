#include "construct-ssa.h"
#include "../control/log.h"
#include "../lib/error.h"
#include "../lib/stack.h"
#include "../lib/trace.h"
#include "../lib/tuple.h"
#include "../lib/unused.h"
#include <assert.h>


static Ssa_Fun_t renameVar(Ssa_Fun_t f, Graph_t g, Tree_t tree);

/////////////////////////////////////////////////////
// properties:
// Ssa_Block_t -> Set<Ssa_Block_t>
// given a block, return its dominance frontier
static Property_t dfProp = 0;
//
//static Set_t dfPropInitFun(Ssa_Block_t b) {
//    UNUSED(b);
//    return Set_new((Poly_tyEquals) Ssa_Block_equals);
//}

// Ssa_Block_t -> Set<Id_t>
// given a block, return set of vars defined in phi-stm
static Property_t phiVarsProp = 0;

static Set_t phiVarsPropInitFun(Ssa_Block_t b) {
    UNUSED(b);

    return Set_new((Poly_tyEquals) Id_equals);
}

// Id_t -> Set<Ssa_Block_t>
// given a var, return a set of blocks that define it
// By default, the set is empty.
static Property_t defSitesProp = 0;

static Set_t defSitesPropInitFun(Id_t id) {
    UNUSED(id);

    return Set_new((Poly_tyEquals) Ssa_Block_equals);
}

// Block_t -> Set<Id_t>
// given a block, return its definition var, don't include
// phi vars.
static Property_t origVarsProp = 0;


static void markDf(Ssa_Block_t b, Set_t set) {
    Property_set(dfProp, b, set);
}


// This function has two tasks:
//   1. Calculate and mark definition blocks for each var, and
//   2. mark each block's orig var.
static Ssa_Block_t markDefSitesOneTraced(Ssa_Block_t b) {
    Set_t defIdSet = Ssa_Block_getDefIds(b);
    List_t defIdsList = List_getFirst(Set_toList(defIdSet));

    // mark the "defIds" on the block "b"
    Property_set(origVarsProp, b, defIdSet);

    while (defIdsList) {
        Id_t id = (Id_t) defIdsList->data;
        Set_t blockSet;

        // if this set is not existed, one empty set will be
        // generated and set on "id".
        blockSet = Property_get(defSitesProp, id);
        Set_insert(blockSet, b);
        defIdsList = defIdsList->next;
    }
    return b;
}

static void dumb(Ssa_Block_t b) {
    UNUSED(b);
}

static void markDefSitesOne(Ssa_Block_t b) {
    Ssa_Block_t r;

    Trace_TRACE("markDefSites", markDefSitesOneTraced, (b), dumb, r, dumb);
    //    return;
}

// This function implements algorithm 19.6 in Appel's book.
static void insertPhiOne(Dec_t dec) {
    Id_t a = dec->id;
    Set_t w = Property_get(defSitesProp, a);

    while (!Set_isEmpty(w)) {
        // "a" is defined in "n"
        Ssa_Block_t n = Set_removeOne(w);
        // "n"'s dominance frontier
        Set_t dfn = Property_get(dfProp, n);
        // to iterate on each "y"
        List_t dfList = List_getFirst(Set_toList(dfn));
        while (dfList) {
            Ssa_Block_t y = (Ssa_Block_t) dfList->data;
            // Set<Id_t>
            Set_t phiVars = Property_get(phiVarsProp, y);
            Set_t origVars = Property_get(origVarsProp, y);

            // if "id" is already added to the block "y" as phi
            if (Set_exists(phiVars, a)) {
                dfList = dfList->next;
                continue;
            }
            Set_insert(phiVars, a);

            if (!Set_exists(origVars, a))
                Set_insert(w, y);

            dfList = dfList->next;
        }// end of inner while
    }// end of outer while
    //    return;
}

static void printDefSites(Dec_t dec) {
    Id_t id = dec->id;
    Set_t blocks = Property_get(defSitesProp, id);
    List_t blockList;


    Log_strs(Id_toString(id), " ==> [", 0);
    blockList = List_getFirst(Set_toList(blocks));

    while (blockList) {
        Ssa_Block_t b = (Ssa_Block_t) blockList->data;

        Log_strs(Label_toString(b->label), ", ", 0);
        blockList = blockList->next;
    }
    Log_str("]");
}

static void printOrigVars(Ssa_Block_t b) {
    Set_t vs = Property_get(origVarsProp, b);
    List_t origVarsList = List_getFirst(Set_toList(vs));


    Log_strs(Label_toString(b->label), " ===> [", 0);
    while (origVarsList) {
        Id_t id = (Id_t) origVarsList->data;
        Log_strs(Id_toString(id), ", ", 0);
        origVarsList = origVarsList->next;
    }
    Log_str("]");
}

static void printPhiVars(Ssa_Block_t b) {
    Set_t vs = Property_get(phiVarsProp, b);
    List_t v = List_getFirst(Set_toList(vs));


    Log_strs(Label_toString(b->label), " ==> [", 0);

    while (v) {
        Id_t id = (Id_t) v->data;

        Log_strs(Id_toString(id), ", ", 0);
        v = v->next;
    }
    Log_str("]");
}

static void printDf(Ssa_Block_t b) {
    Set_t df = Property_get(dfProp, b);
    List_t dfList = List_getFirst(Set_toList(df));

    Log_strs(Label_toString(b->label), " ==> [", 0);

    while (dfList) {
        Ssa_Block_t x = (Ssa_Block_t) dfList->data;

        Log_strs(Label_toString(x->label), ", ", 0);
        dfList = dfList->next;
    }
    Log_str("]");
    //    return;
}

static List_t realInsertPhi(List_t bs, Graph_t g) {
    List_t blocks = List_getFirst(bs);
    List_t newBlocks = List_new();

    while (blocks) {
        Ssa_Block_t b = (Ssa_Block_t) blocks->data;
        List_t phiStms = List_new();
        Ssa_Block_t fresh;
        Set_t phiVars = Property_get(phiVarsProp,
                                     b);
        List_t phiVarsList = List_getFirst(Set_toList(phiVars));

        while (phiVarsList) {
            Id_t var = (Id_t) phiVarsList->data;
            List_t preds = Graph_predessors(g, b);
            Ssa_Stm_t phi = Ssa_Stm_new_phi_predsBlock(var, preds);

            List_insertLast(phiStms, phi);
            phiVarsList = phiVarsList->next;
        }
        List_append(phiStms, b->stms);
        fresh = Ssa_Block_new(b->label, phiStms, b->transfer);
        List_insertLast(newBlocks, fresh);
        blocks = blocks->next;
    }
    return newBlocks;
}

static Ssa_Fun_t insertPhiAndRename(Ssa_Fun_t f) {
    List_t blocks;
    // f's control flow graph
    Graph_t g;
    Tree_t tree;
    Ssa_Fun_t tempf;

    assert(f);

    blocks = f->blocks;

    // map every id to a set of its definition block
    // Id_t -> Set_t<Ssa_Block_t>
    defSitesProp = Property_newInitFun((Poly_tyPlist) Id_plist, (Poly_tyPropInit) defSitesPropInitFun);

    // a set of vars that originially defined in a block
    // excluding phis
    origVarsProp = Property_new((Poly_tyPlist) Ssa_Block_plist);

    // two jobs:
    //   1. mark each var for its definition blocks, and
    //   2. mark each block's orig vars.
    List_foreach(blocks, (Poly_tyVoid) markDefSitesOne);

    // now the defsites for each id and orig vars for each
    // block should have been set.
    Log_str("check def sites:");
    List_foreach(f->args, (Poly_tyVoid) printDefSites);
    List_foreach(f->decs, (Poly_tyVoid) printDefSites);

    Log_str("check orig vars:");
    List_foreach(blocks, (Poly_tyVoid) printOrigVars);

    // insert phi
    // this is the phi-vars on every block
    phiVarsProp = Property_newInitFun((Poly_tyPlist) Ssa_Block_plist, (Poly_tyPropInit) phiVarsPropInitFun);

    dfProp = Property_new((Poly_tyPlist) Ssa_Block_plist);

    g = Ssa_Fun_toGraph(f);
    Graph_df(g, (Poly_t) Ssa_Fun_searchLabel(f, f->entry), (void (*)(Poly_t, Set_t)) markDf);

    Log_str("checking dominance frontier starting:");
    List_foreach(blocks, (Poly_tyVoid) printDf);

    List_foreach(f->args, (Poly_tyVoid) insertPhiOne);
    List_foreach(f->decs, (Poly_tyVoid) insertPhiOne);

    Log_str("check phi arg:");
    List_foreach(blocks, (Poly_tyVoid) printPhiVars);

    // the real insertion starts here
    blocks = realInsertPhi(blocks, g);

    // must rebuild the graph and the tree:
    tempf = Ssa_Fun_new(f->type, f->name, f->args, f->decs, blocks, f->retId, f->entry, f->exitt);

    g = Ssa_Fun_toGraph(tempf);
    tree = Graph_df(g, (Poly_t) Ssa_Fun_searchLabel(f, f->entry), (void (*)(Poly_t, Set_t)) markDf);

    Log_strs("num of blocks in this function =", Int_toString(List_size(blocks)), "\n", 0);

    // check point
    Log_dot((Poly_tyDot) Ssa_Fun_toDot, tempf, "insertphi");
    File_saveToFile("insertPhi", (Poly_tyPrint) Ssa_Fun_print, tempf);
    //Graph_toJpgWithName (g, Ssa_Block_printForDot, "graph");
    //Tree_toJpgWithName (tree, Ssa_Block_printForDot, "tree");
    //

    Log_str("rename var starting\n");
    tempf = renameVar(tempf, g, tree);
    Log_str("rename var finished\n");


    Log_dot((Poly_tyDot) Ssa_Fun_toDot, tempf, "rename");

    // clear properties, excluding the "phiVarsProp".
    Property_clear(defSitesProp);
    Property_clear(origVarsProp);
    Property_clear(phiVarsProp);
    Property_clear(dfProp);
    defSitesProp = 0;
    origVarsProp = 0;
    phiVarsProp = 0;
    dfProp = 0;

    return tempf;
}

/////////////////////////////////////////////////////
// the following two properties are renaming-related.
// per-id stack.
// Id_t -> Stack_t<Id>
static Property_t stackProp = 0;

static Stack_t stackPropInitFun(Id_t id) {
    Stack_t stack = Stack_new();
    Stack_push(stack, id);
    return stack;
}

// Block_t -> Block_t
// store every block's new block, this is nearly the
// final result, except for that the arguments in phis
// should be further elaborated.
static Property_t substProp = 0;

// Block_t -> List<Triple<Id_t, Block_t, Id_t>>
// remember every old phi argument to the new one.
static Property_t substPhiProp = 0;

static List_t substPhiPropInitFun(Ssa_Block_t b) {
    UNUSED(b);

    return List_new();
}

// Id_t -> List<Id_t>
// For every id, remember all its new names (versions).
static Property_t freshNameProp = 0;

static List_t freshNamePropInitFun(Id_t id) {
    UNUSED(id);
    return List_new();
}

static Id_t use(Id_t id) {
    Stack_t stk = Property_get(stackProp, id);

    Id_t newid;

    if (Stack_isEmpty(stk)) {
        printf("id = %s\n", Id_toString(id));
        Log_reset();
        printf("empty stack on id: %s\n", Id_toString(id));
        Error_impossible();
    }
    newid = Stack_getTop(stk);
    return newid;
}

static void callBackSubstPhiProp(Ssa_Block_t current, Ssa_Block_t pred, Id_t oldid) {
    Id_t new = use(oldid);
    Ssa_Stm_PhiArg_t arg = Ssa_Stm_PhiArg_new(Ssa_Operand_new_id(oldid), pred);
    Tuple_t t = Tuple_new(arg, new);
    List_t l = Property_get(substPhiProp, current);
    Log_strs("record (", Id_toString(oldid), ", ", Label_toString(pred->label), ") ~~~~> ", Id_toString(new), " on ",
             Label_toString(current->label), "\n", 0);
    List_insertFirst(l, t);
}

static Graph_t theg = 0;
static Tree_t thetree = 0;

static Id_t def(Id_t id) {
    Id_t fresh = Id_newNoName();
    Stack_t stk = Property_get(stackProp, id);
    List_t freshNames = Property_get(freshNameProp, id);

    // push the new name onto the stack
    Stack_push(stk, fresh);
    // and remember it
    List_insertLast(freshNames, fresh);
    return fresh;
}

static void popDef(Id_t id) {
    Stack_t stk = Property_get(stackProp, id);
    Stack_pop(stk);
}

// the block "n" is new, whereas the graph "theg" and
// the dominator tree "thetree" are old.
static void renameVarDoit(Ssa_Block_t n) {
    // rename each use according to "use", rename each
    // definition
    // according to "def". Including defs in phi but
    // excluding uses in phi.
    Ssa_Block_t newBlock;

    Log_str("renameVarDoit starting\n");
    Log_fun(n, (Poly_tyPrint) Ssa_Block_print);
    newBlock = Ssa_Block_renameUseDefNoPhiUse(n, use, def);

    Log_str("the new block after renaming defuse:");
    //
    Log_fun(newBlock, (Poly_tyPrint) Ssa_Block_print);
    Log_str("end of the new block after renaming defuse:");

    Property_set(substProp, n, newBlock);

    // for each successor y of n
    {
        List_t succs = Graph_successors(theg, n);

        succs = List_getFirst(succs);
        while (succs) {
            Ssa_Block_t y = (Ssa_Block_t) succs->data;

            Ssa_Block_renamePhiArgPre(y, n, callBackSubstPhiProp);
            succs = succs->next;
        }
    }
    Log_str("dominator tree children:");
    // for each child
    {
        List_t children = Tree_children(thetree, n);
        children = List_getFirst(children);
        while (children) {
            Ssa_Block_t x = (Ssa_Block_t) children->data;

            renameVarDoit(x);
            children = children->next;
        }
    }
    Log_str("exit from one tree children\n");
    // for each statement (including phi and transfer)
    Ssa_Block_foreachDef(n, popDef);
    Log_str("renameVarEach finished");
}

static void checkPhiArgs(Ssa_Block_t b, List_t l) {
    Log_str("checking phi args starting:");
    Log_strs(Label_toString(b->label), " ===>(", Int_toString(List_size(l)), "):\n", 0);

    l = List_getFirst(l);
    while (l) {
        Tuple_t t = (Tuple_t) l->data;
        Ssa_Stm_PhiArg_t arg = Tuple_first(t);
        Id_t new = Tuple_second(t);

        Log_strs("\t ", Ssa_Stm_PhiArg_toString(arg), 0);
        Log_strs(" ~~~~> ", Id_toString(new), "\n", 0);
        l = l->next;
    }
    Log_str("checking phi args finished");
}

static Ssa_Block_t rewriteBlock(Ssa_Block_t b) {
    // get the new block from the old one
    Ssa_Block_t newb = Property_get(substProp, b);
    List_t phiargs = Property_get(substPhiProp, b);

    // before going on, checking this.
    checkPhiArgs(b, phiargs);

    newb = Ssa_Block_renamePhiArgs(newb, phiargs);
    return newb;
}

// whether or not we should copy the old ones
static int shouldcopy = 0;

static List_t genNewVarsEach(List_t result, Dec_t dec) {
    Id_t old = dec->id;
    Atype_t ty = dec->ty;

    List_t freshVars = Property_get(freshNameProp, old);
    freshVars = List_getFirst(freshVars);
    if (shouldcopy)
        List_insertLast(result, dec);
    while (freshVars) {
        Id_t fresh = freshVars->data;

        Dec_t freshDec = Dec_new(ty, fresh);
        List_insertLast(result, freshDec);
        freshVars = freshVars->next;
    }
    return result;
}

static List_t genNewVars(List_t args, List_t decs) {
    List_t result = List_new();

    shouldcopy = 0;
    List_foldl(args, result, (Poly_t(*)(Poly_t, Poly_t)) genNewVarsEach);
    shouldcopy = 1;
    List_foldl(decs, result, (Poly_t(*)(Poly_t, Poly_t)) genNewVarsEach);

    return result;
}

// This function implements algorithm 19.7 on Appel's book.
// Here, the blocks are new, whereas the "g" and "tree" are
// old in the sense that they have no phis.
static Ssa_Fun_t renameVarTraced(Ssa_Fun_t f, Graph_t g, Tree_t tree) {
    List_t blocks = f->blocks;
    List_t newBlocks;
    Ssa_Fun_t newf;
    List_t newDecs;

    stackProp = Property_newInitFun((Poly_tyPlist) Id_plist, (Poly_tyPropInit) stackPropInitFun);
    substProp = Property_new((Poly_tyPlist) Ssa_Block_plist);
    substPhiProp = Property_newInitFun((Poly_tyPlist) Ssa_Block_plist, (Poly_tyPropInit) substPhiPropInitFun);

    freshNameProp = Property_newInitFun((Poly_tyPlist) Id_plist, (Poly_tyPropInit) freshNamePropInitFun);

    Log_str("renameVar starting:");
    theg = g;
    thetree = tree;

    Log_str("the initial function is:");
    Log_dot((Poly_tyDot) Ssa_Fun_toDot, f, "init");

    Log_str("before renameVarDoit ()\n");
    renameVarDoit(Ssa_Fun_searchLabel(f, f->entry));
    Log_dot((Poly_tyDot) Ssa_Fun_toDot, f, "2");
    theg = 0;
    thetree = 0;

    Log_str("before rewriteblocks");
    newBlocks = List_map(blocks, (Poly_tyId) rewriteBlock);
    Log_str("renameVar finished");


    Log_str("generating fresh vars starting:");

    newDecs = genNewVars(f->args, f->decs);

    Property_clear(substProp);
    Property_clear(substPhiProp);
    Property_clear(freshNameProp);

    newf = Ssa_Fun_new(f->type, f->name, f->args, newDecs, newBlocks, f->retId, f->entry, f->exitt);
    return newf;
}

static void dumb1(Ssa_Fun_t f, Graph_t g, Tree_t tree) {
    UNUSED(f);
    UNUSED(g);
    UNUSED(tree);
}

static void dumb2(Ssa_Fun_t f) {
    UNUSED(f);
}

static Ssa_Fun_t renameVar(Ssa_Fun_t f, Graph_t g, Tree_t tree) {
    Ssa_Fun_t r;

    Trace_TRACE("renameVar", renameVarTraced, (f, g, tree), dumb1, r, dumb2);
    return r;
}

//////////////////////////////////////////////////////
// functions
static Ssa_Fun_t transFunEach(Ssa_Fun_t f) {
    //    Ssa_Block_t eb;// entry block

    assert(f);
    //Log_dot (Ssa_Fun_toDot, f, "raw");
    Log_strs("now ready to translate function: ", Id_toString(f->name), "\n", 0);
    f = insertPhiAndRename(f);
    Log_strs("translating function finished: ", Id_toString(f->name), "\n", 0);
    return f;
}


//////////////////////////////////////////////////////
// programs
static Ssa_Prog_t Ssa_constructSsaTraced(Ssa_Prog_t p) {
    List_t newFuncs;

    assert(p);
    newFuncs = List_map(p->funcs, (Poly_tyId) transFunEach);
    return Ssa_Prog_new(p->classes, newFuncs);
}

/////////////////////////////////////////////////////
// main functions
static void printArg(Ssa_Prog_t p) {
    Ssa_Prog_toDot(p, "beforeConsSsa");
    File_saveToFile("consSsa.arg", (Poly_tyPrint) Ssa_Prog_print, p);
}

static void printResult(Ssa_Prog_t p) {
    //Ssa_Prog_toDot (p, "afterConsSsa");
    // and also print it out
    File_saveToFile("consSsa.result", (Poly_tyPrint) Ssa_Prog_print, p);
}

Ssa_Prog_t Ssa_constructSsa(Ssa_Prog_t p) {
    Ssa_Prog_t r;

    Log_POS();
    Trace_TRACE("Ssa_constructSsa", Ssa_constructSsaTraced, (p), printArg, r, printResult);
    return r;
}
