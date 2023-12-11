#ifndef ENV_H
#define ENV_H

#include "../ast/ast-id.h"
#include "type.h"

/////////////////////////////////////////////////////
// Env binding. For "Venv" and "Senv".
typedef struct {
    Type_t type;
    AstId_t fresh;
} *Env_Binding_t;

Env_Binding_t Env_Binding_new(Type_t, AstId_t);

///////////////////////////////////////////////////////
// ClassName env.
/* This env stores all
 * defined class names. This is used in the 1st pass
 * scan of all classes. The result Id_t is the fresh
 * name the class is assigned.
 * 
 *   Cenv: Id_t -> {Id_t, List_t}
 */
void Cenv_init(void);

// insert a new class name and it's fields into Cenv.
// If a name already exist, report errors.
AstId_t Cenv_insert(AstId_t id, List_t);

// peek. Slient for non-existing names.
AstId_t Cenv_lookup(AstId_t id);

// peek. Complain for non-existing names.
AstId_t Cenv_lookupMustExist(AstId_t);

// peek class fields, must exist
List_t Cenv_lookupFields(AstId_t);

///////////////////////////////////////////////////////
/* variable env: 
 *   Stack<Hash<Id_t, Type_t>> 
 */
void Venv_init(void);

AstId_t Venv_insert(AstId_t id, Type_t type);

Env_Binding_t Venv_lookup(AstId_t);

Env_Binding_t Venv_lookupMustExist(AstId_t);

void Venv_enterScope(void);

void Venv_exitScope(void);

///////////////////////////////////////////////////////
/* Class field env:
 *   Id_t * Id_t -> Id_t
 */
void Senv_init(void);

AstId_t Senv_insert(AstId_t sid, AstId_t fid, Type_t ty);

Env_Binding_t Senv_lookupMustExist(AstId_t sid, AstId_t fid);

#endif
