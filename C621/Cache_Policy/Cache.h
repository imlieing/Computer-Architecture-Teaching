#ifndef __CACHE_H__
#define __CACHE_H__

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <stdint.h>

#include "Cache_Blk.h"
#include "Request.h"

// #define LRU
// #define LFU
#define ARC

/* Cache */
typedef struct Set
{
    Cache_Block **ways; // Block ways within a set
}Set;

typedef struct Cache
{
    uint64_t blk_mask;
    unsigned num_blocks;
    
    Cache_Block *blocks; // All cache blocks

    /* Set-Associative Information */
    unsigned num_sets; // Number of sets
    unsigned num_ways; // Number of ways within a set

    unsigned set_shift;
    unsigned set_mask; // To extract set index
    unsigned tag_shift; // To extract tag

    Set *sets; // All the sets of a cache
    
}Cache;

typedef struct ARCCache
{
    uint64_t blk_mask;
    unsigned tag_shift; // To extract tag
    unsigned set_shift;

    int p;

    Cache_Block **l1; // All cache blocks
    Cache_Block **l2; // All cache blocks
}ARCCache;

// Function Definitions
Cache *initCache();
bool accessBlock(Cache *cache, Request *req, uint64_t access_time);
bool insertBlock(Cache *cache, Request *req, uint64_t access_time, uint64_t *wb_addr);
// Helper Function
uint64_t blkAlign(uint64_t addr, uint64_t mask);
Cache_Block *findBlock(Cache *cache, uint64_t addr);

ARCCache *initARCCache();
bool accessBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time);
// bool insertBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time, uint64_t *wb_addr);
Cache_Block *findBlockARC(ARCCache *arc_cache, uint64_t addr);


// Replacement Policies
bool lru(Cache *cache, uint64_t addr, Cache_Block **victim_blk);
bool lfu(Cache *cache, uint64_t addr, Cache_Block **victim_blk, uint64_t *wb_addr);
bool arc(ARCCache *arc_cache, uint64_t addr, Cache_Block **victim_blk);

#endif
