#include "allocator.h"
#include "cfe.h"

#ifdef RTEMS_BUILD
#include <rtems.h>
#define MPA_HALT(fmt, ...) do { printf("mempool_alloc: " fmt "\n", ##__VA_ARGS__); \
                               rtems_fatal(RTEMS_FATAL_SOURCE_APPLICATION, 1); } while (0)
#else
#include <stdio.h>
#include <stdlib.h>
#define MPA_HALT(fmt, ...) do { fprintf(stderr, "mempool_alloc: " fmt "\n", ##__VA_ARGS__); \
                               abort(); } while (0)
#endif

typedef struct {
    HAllocator  base; //keep first
    // cFE MemPool Handle
    CFE_ES_MemHandle_t  PoolId;
    uint8_t    *buf;
} MemPoolAllocator;

static MemPoolAllocator grammar_mpa;
static MemPoolAllocator parse_mpa;

/* Public handles callers reference. Set in h_static_alloc_init. */
HAllocator *h_grammar_allocator_mpa;
HAllocator *h_parse_allocator_mpa;


#if defined(DEBUG__MEMFILL)
/**
 * Blocks allocated by the system_allocator start with this header.
 * I.e. the user part of the allocation directly follows.
 */
typedef struct HDebugBlockHeader_ {
    size_t size; /** size of the user allocation */
} HDebugBlockHeader;

#define BLOCK_HEADER_SIZE (sizeof(HDebugBlockHeader))
#else
#define BLOCK_HEADER_SIZE (0)
#endif

/**
 * Compute the total size needed for a given allocation size.
 */
static inline size_t block_size(size_t alloc_size) { return BLOCK_HEADER_SIZE + alloc_size; }

/**
 * Obtain the block containing the user pointer `uptr`
 */
static inline void *block_for_user_ptr(void *uptr) { return ((char *)uptr) - BLOCK_HEADER_SIZE; }

/**
 * Obtain the user area of the allocation from a given block
 */
static inline void *user_ptr(void *block) { return ((char *)block) + BLOCK_HEADER_SIZE; }

/* ---- HAllocator vtable ---- */

static void *mempool_alloc(HAllocator *allocator, size_t size) {
    MemPoolAllocator *a = (MemPoolAllocator *)allocator;
    // ensure that usable ptr is returned in rtems build
    #ifdef RTEMS_BUILD
    if(size ==0) size=1;
    #endif // #ifdef RTEMS_BUILD
    void *block;
    CFE_ES_GetPoolBuf((void **)&block, a->PoolId, block_size(size));
    // void *block = malloc(block_size(size));
    if (!block) {
        return NULL;
    }
    void *uptr = user_ptr(block);
#ifdef DEBUG__MEMFILL
    memset(uptr, DEBUG__MEMFILL, size);
    ((HDebugBlockHeader *)block)->size = size;
#endif
    return uptr;
}

static void mempool_free(HAllocator *allocator, void *uptr) {
    MemPoolAllocator *a = (MemPoolAllocator *)allocator;
    if (uptr) {
        CFE_ES_PutPoolBuf(a->PoolId, (uint32 *)uptr);
        // free(block_for_user_ptr(uptr));
    }
}

static void *mempool_realloc(HAllocator *mm, void *ptr, size_t size) {
    MemPoolAllocator *a = (MemPoolAllocator *)mm;
    (void)ptr;
    (void)size;
    // Not reachable on the single-shot h_parse path. If we get here, the
    // streaming (chunked) path was used unexpectedly. Fail
    MPA_HALT("realloc called but not supported on this path");
    return NULL; // unreachable
}

void h_mempool_alloc_init(CFE_ES_MemHandle_t PoolId, uint8* MemBuf) {

    grammar_mpa.base.alloc   = mempool_alloc;
    grammar_mpa.base.realloc = mempool_realloc;
    grammar_mpa.base.free    = mempool_free;
    grammar_mpa.PoolId       = PoolId;
    grammar_mpa.buf = MemBuf;


    parse_mpa.base.alloc     = mempool_alloc;
    parse_mpa.base.realloc   = mempool_realloc;
    parse_mpa.base.free      = mempool_free;
    parse_mpa.PoolId         = PoolId;
    parse_mpa.buf = MemBuf;

    h_grammar_allocator_mpa = &grammar_mpa.base;
    h_parse_allocator_mpa   = &parse_mpa.base;
}
