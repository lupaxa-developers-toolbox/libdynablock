/* Allocation pool.
   The public contract is in include/dynablock.h. This file holds the pool
   and block layout. Each block is one allocation: a header, then a payload
   of the exact requested size. The payload is aligned for any object. The
   caller receives the payload. The pool keeps the headers in a list so one
   block, or every block, can be freed. */

/* Quoted path so a syntax check without -I still finds the public header. */
#include "../include/dynablock.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* One owned allocation. next links the pool list. bytes is the payload
   size. The payload begins header_bytes() after this header. */
struct block {
    struct block *next;
    size_t bytes;
};

/* Pool header. head is NULL when the pool holds no blocks. count is the
   number of blocks still owned. */
struct dynablock {
    struct block *head;
    size_t count;
};

/* Bytes from the start of a block to its payload, rounded up so the
   payload is aligned for max_align_t. */
static size_t header_bytes(void)
{
    size_t raw = sizeof(struct block);
    size_t align = alignof(max_align_t);

    return (raw + (align - 1)) & ~(align - 1);
}

static void *payload_of(struct block *block)
{
    return (unsigned char *)block + header_bytes();
}

/* Find the block whose payload is block. A pointer this pool does not own
   returns NULL. The search compares payload addresses, so a foreign pointer
   is never treated as a header. */
static struct block *find_block(const dynablock *pool, const void *block,
                                struct block **previous)
{
    struct block *prev = NULL;
    struct block *cursor;

    for (cursor = pool->head; cursor != NULL; cursor = cursor->next) {
        if (payload_of(cursor) == block) {
            if (previous != NULL) {
                *previous = prev;
            }
            return cursor;
        }
        prev = cursor;
    }
    return NULL;
}

/* Create an empty pool. On allocation failure *pool is NULL. */
int dynablock_create(dynablock **pool)
{
    dynablock *created;

    if (pool == NULL) {
        return DYNABLOCK_INVAL;
    }

    created = calloc(1, sizeof(*created));
    if (created == NULL) {
        *pool = NULL;
        return DYNABLOCK_NOMEM;
    }

    *pool = created;
    return DYNABLOCK_OK;
}

/* Free every block, then the pool. *pool is set to NULL. */
int dynablock_destroy(dynablock **pool)
{
    struct block *block;
    struct block *next;

    if (pool == NULL) {
        return DYNABLOCK_INVAL;
    }
    if (*pool == NULL) {
        return DYNABLOCK_OK;
    }

    block = (*pool)->head;
    while (block != NULL) {
        next = block->next;
        free(block);
        block = next;
    }

    free(*pool);
    *pool = NULL;
    return DYNABLOCK_OK;
}

/* Allocate one zeroed payload and link its header at the head. A zero size
   or a size that cannot fit beside the header returns DYNABLOCK_INVAL and
   leaves *block unchanged. Allocation failure does the same. */
int dynablock_get(dynablock *pool, size_t bytes, void **block)
{
    size_t header;
    struct block *created;

    if (pool == NULL || block == NULL || bytes == 0) {
        return DYNABLOCK_INVAL;
    }

    header = header_bytes();
    if (bytes > SIZE_MAX - header) {
        return DYNABLOCK_INVAL;
    }

    created = calloc(1, header + bytes);
    if (created == NULL) {
        return DYNABLOCK_NOMEM;
    }

    created->next = pool->head;
    created->bytes = bytes;
    pool->head = created;
    pool->count++;
    *block = payload_of(created);
    return DYNABLOCK_OK;
}

/* Unlink and free a payload this pool owns. Anything else is left in place. */
int dynablock_release(dynablock *pool, void *block)
{
    struct block *previous = NULL;
    struct block *owned;

    if (pool == NULL || block == NULL) {
        return DYNABLOCK_INVAL;
    }

    owned = find_block(pool, block, &previous);
    if (owned == NULL) {
        return DYNABLOCK_INVAL;
    }

    if (previous == NULL) {
        pool->head = owned->next;
    } else {
        previous->next = owned->next;
    }
    pool->count--;
    free(owned);
    return DYNABLOCK_OK;
}

size_t dynablock_count(const dynablock *pool)
{
    if (pool == NULL) {
        return 0;
    }
    return pool->count;
}
