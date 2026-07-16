#ifndef HAMMER_MEMPOOL_ALLOC_H
#define HAMMER_MEMPOOL_ALLOC_H

#include "allocator.h"   /* HAllocator */
#include "cfe.h"

extern HAllocator *h_grammar_allocator_mpa;
extern HAllocator *h_parse_allocator_mpa;

void h_mempool_alloc_init(CFE_ES_MemHandle_t  PoolId);

#endif /* HAMMER_MEMPOOL_ALLOC_H */