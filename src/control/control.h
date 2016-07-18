#ifndef CONTROL_H
#define CONTROL_H

#include "../lib/list.h"
#include "../lib/string.h"

typedef enum {VERBOSE_SILENT, 
              VERBOSE_PASS, 
              VERBOSE_SUBPASS, 
              VERBOSE_DETAIL} Verbose_t;

typedef enum {EXPERT_NORMAL, 
              EXPERT_EXPERT} Expert_t;

typedef enum {DUMP_AST,
              DUMP_HIL,
              DUMP_TAC,
              DUMP_MACHINE,
              DUMP_C
              , DUMP_X86} Dump_t;

typedef enum{
  CODEGEN_C
  , CODEGEN_X86
}Codegen_t;

extern int Control_bufferSize;
extern Codegen_t Control_codegen;
extern Expert_t Control_expert;
extern int Control_labelInfo;
// show type information in ILs
extern int Control_showType;
extern List_t Control_trace;
extern Verbose_t Control_verbose;
extern String_t Control_o;

// keep jpg, should be a more
extern int Control_jpg;

/* this is platform dependent, so should be 
 * sent to another place.
 */
extern String_t Control_asmDirectory;
extern String_t Control_libDirectory;
extern String_t Control_headerDirectory;

int Control_Verb_order (Verbose_t v1, Verbose_t v2);
void Control_dump_insert (Dump_t);
int Control_dump_lookup (Dump_t);
// drop pass
int Control_mayDropPass (String_t name);
void Control_dropPass_insert (String_t name);

// log pass
int Control_logPass (String_t passname);
void Control_logPass_insert (String_t passname);

// code-generation-related
extern int Control_Target_size;
//
void Control_init ();
void Control_printFlags ();



#endif
