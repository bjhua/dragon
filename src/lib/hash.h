#ifndef HASH_H
#define HASH_H

#include "list.h"
#include "poly.h"
#include "string.h"

#define T Hash_t
#define K Poly_t
#define V Poly_t

typedef struct T *T;

typedef V (*tyKV)(K);

typedef void (*tyDup)(K, K);

T Hash_new(long (*hashCode)(K), long (*equals)(K, K), void (*dup)(K, K));

/* insert a binding: k |-> v into a hash h,
 * with the equality testing function "equals".
 * if such a binding already exists, then call dup(k).
 */
void Hash_insert(T h, K k, V v);


V Hash_lookup(T h, K k);
// Just behave as "lookup", but return the
// found key in "*result" (if result!=0).
V Hash_lookupCand(T h, K k, K *result);
/* lookup the key "k" in the hash "h".
 * if the item not found, then call gen(k) to generate 
 * a value insert the binding "k->v" into the hash 
 * before returning v.
 */
V Hash_lookupOrInsert(T h,
                      K k,
                      V (*gen)(K));

/* delete a key k from a hash. Error if k not exists,
 * call notFound (k).
 */
void Hash_delete(T h, K x);

// apply f to each k in h.
void Hash_foreach(T h, void (*f)(K));

double Hash_loadFactor(T h);

String_t Hash_status(T h);

void Hash_statusAll(void);

long Hash_size(T h);

long Hash_numItems(T h);

// turn key into a list.
List_t Hash_keyToList(T h);

#undef K
#undef V
#undef T

#endif
