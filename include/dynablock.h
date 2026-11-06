#ifndef DYNABLOCK_H
#define DYNABLOCK_H

#include <stddef.h>

#define DYNABLOCK_VERSION_STRING "0.1.0"

/* DYNABLOCK_OK is success. DYNABLOCK_NOMEM means allocation failed and the
   pool was left unchanged. dynablock_create sets *pool to NULL on failure.
   DYNABLOCK_INVAL means a missing pool, a missing out-pointer, a zero size,
   or a block that is not in that pool. */
#define DYNABLOCK_OK 0
#define DYNABLOCK_NOMEM (-1)
#define DYNABLOCK_INVAL (-2)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dynablock dynablock;

/* Allocate an empty pool. On success *pool receives it. On allocation
   failure *pool is NULL. */
int dynablock_create(dynablock **pool);

/* Free every block still in the pool, then free the pool. *pool is set to
   NULL. Destroying an already NULL pool is success. */
int dynablock_destroy(dynablock **pool);

/* Allocate a zeroed block of exactly bytes bytes and store the payload in
   *block. bytes must be at least 1. On failure the pool and *block are
   unchanged. The pointer is invalid after dynablock_release or
   dynablock_destroy. */
int dynablock_get(dynablock *pool, size_t bytes, void **block);

/* Free one block that belongs to this pool. A block from another pool, or
   a pointer this pool does not own, returns DYNABLOCK_INVAL and frees
   nothing. block is invalid after success. */
int dynablock_release(dynablock *pool, void *block);

size_t dynablock_count(const dynablock *pool);

#ifdef __cplusplus
}
#endif

#endif /* DYNABLOCK_H */
