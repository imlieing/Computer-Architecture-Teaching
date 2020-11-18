#include "Cache.h"

/* Constants */
const unsigned block_size = 64; // Size of a cache line (in Bytes)
// TODO, you should try different size of cache, for example, 128KB, 256KB, 512KB, 1MB, 2MB
const unsigned cache_size = 2048; // Size of a cache (in KB)
// TODO, you should try different association configurations, for example 4, 8, 16
const unsigned assoc = 4;

Cache *initCache()
{
    printf("\n");
    printf("cache_size: %u\n", cache_size);
    printf("assoc: %u\n", assoc);
    Cache *cache = (Cache *)malloc(sizeof(Cache));

    cache->blk_mask = block_size - 1;

    unsigned num_blocks = cache_size * 1024 / block_size;
    cache->num_blocks = num_blocks;
//    printf("Num of blocks: %u\n", cache->num_blocks);

    // Initialize all cache blocks
    cache->blocks = (Cache_Block *)malloc(num_blocks * sizeof(Cache_Block));
    
    int i;
    for (i = 0; i < num_blocks; i++)
    {
        cache->blocks[i].tag = UINTMAX_MAX; 
        cache->blocks[i].valid = false;
        cache->blocks[i].dirty = false;
        cache->blocks[i].when_touched = 0;
        cache->blocks[i].frequency = 0;
    }

    // Initialize Set-way variables
    unsigned num_sets = cache_size * 1024 / (block_size * assoc);
    cache->num_sets = num_sets;
    cache->num_ways = assoc;
//    printf("Num of sets: %u\n", cache->num_sets);

    unsigned set_shift = log2(block_size);
    cache->set_shift = set_shift;
//    printf("Set shift: %u\n", cache->set_shift);

    unsigned set_mask = num_sets - 1;
    cache->set_mask = set_mask;
//    printf("Set mask: %u\n", cache->set_mask);

    unsigned tag_shift = set_shift + log2(num_sets);
    cache->tag_shift = tag_shift;
//    printf("Tag shift: %u\n", cache->tag_shift);

    // Initialize Sets
    cache->sets = (Set *)malloc(num_sets * sizeof(Set));
    for (i = 0; i < num_sets; i++)
    {
        cache->sets[i].ways = (Cache_Block **)malloc(assoc * sizeof(Cache_Block *));
    }

    // Combine sets and blocks
    for (i = 0; i < num_blocks; i++)
    {
        Cache_Block *blk = &(cache->blocks[i]);
        
        uint32_t set = i / assoc;
        uint32_t way = i % assoc;

        blk->set = set;
        blk->way = way;

        cache->sets[set].ways[way] = blk;
    }

    return cache;
}
ARCCache *initARCCache()
{
    int i;
    ARCCache *arc_cache = (ARCCache *)malloc(sizeof(ARCCache));
    Cache_Block **l1 = (Cache_Block **)malloc(cache_size * sizeof(Cache_Block*));
    Cache_Block **l2 = (Cache_Block **)malloc(cache_size * sizeof(Cache_Block*));

    for (i = 0; i < cache_size; i++)
    {
        l1[i] = (Cache_Block *)malloc(sizeof(Cache_Block));
        l2[i] = (Cache_Block *)malloc(sizeof(Cache_Block));

        l1[i]->tag = UINTMAX_MAX;
        l1[i]->valid = false;
        l1[i]->dirty = false;
        l1[i]->when_touched = 0;
        l1[i]->frequency = 0;

        l2[i]->tag = UINTMAX_MAX;
        l2[i]->valid = false;
        l2[i]->dirty = false;
        l2[i]->when_touched = 0;
        l2[i]->frequency = 0;
    }
    arc_cache->l1 = l1;
    arc_cache->l2 = l2;
    arc_cache->blk_mask = block_size - 1;
    arc_cache->p = 0;
    
    unsigned num_sets = cache_size * 1024 / (block_size * assoc);
    unsigned set_shift = log2(block_size);
    unsigned tag_shift = set_shift + log2(num_sets);
    arc_cache->set_shift = set_shift;
    arc_cache->tag_shift = tag_shift;
    return arc_cache;
}

bool accessBlock(Cache *cache, Request *req, uint64_t access_time)
{
    bool hit = false;

    uint64_t blk_aligned_addr = blkAlign(req->load_or_store_addr, cache->blk_mask);

    Cache_Block *blk = findBlock(cache, blk_aligned_addr);
   
    if (blk != NULL) 
    {
        hit = true;

        // Update access time	
        blk->when_touched = access_time;
        // Increment frequency counter
        ++blk->frequency;

        if (req->req_type == STORE)
        {
            blk->dirty = true;
        }
    }

    return hit;
}

bool insertBlock(Cache *cache, Request *req, uint64_t access_time, uint64_t *wb_addr)
{
    // Step one, find a victim block
    uint64_t blk_aligned_addr = blkAlign(req->load_or_store_addr, cache->blk_mask);

    Cache_Block *victim = NULL;
    bool wb_required = true; 
    #ifdef LRU
        wb_required = lru(cache, blk_aligned_addr, &victim);
    #endif

    #ifdef LFU
        wb_required = lfu(cache, blk_aligned_addr, &victim, wb_addr);
    #endif

    assert(victim != NULL);

    // Step two, insert the new block
    uint64_t tag = req->load_or_store_addr >> cache->tag_shift;
    victim->tag = tag;
    victim->valid = true;

    victim->when_touched = access_time;
    ++victim->frequency;

    if (req->req_type == STORE)
    {
        victim->dirty = true;
    }

    return wb_required;
//    printf("Inserted: %"PRIu64"\n", req->load_or_store_addr);
}

Cache_Block *findBlock(Cache *cache, uint64_t addr)
{
    uint64_t tag = addr >> cache->tag_shift;
    uint64_t set_idx = (addr >> cache->set_shift) & cache->set_mask;
    Cache_Block **ways = cache->sets[set_idx].ways;
    int i;
    for (i = 0; i < cache->num_ways; i++)
    {
        if (tag == ways[i]->tag && ways[i]->valid == true)
        {
            return ways[i];
        }
    }

    return NULL;
}

void shiftL1Down1Index(ARCCache* arc_cache)
{
    int i = 0;
    for (i = cache_size; i>0; i-- )
    {
        arc_cache->l1[i] = arc_cache->l1[i-1];
    }
}

void shiftL2Down1Index(ARCCache* arc_cache)
{
    int i = 0;
    for (i = cache_size; i>0; i-- )
    {
        arc_cache->l2[i] = arc_cache->l2[i-1];
    }
}



Cache_Block *findBlockARC(ARCCache *arc_cache, uint64_t addr)
{
    uint64_t tag = addr >> arc_cache->tag_shift;
    int i;
    int j;
    bool found;
    for (i = 0; i < cache_size; i++)
    {
        if(found){break;}
        if (tag==arc_cache->l1[i]->tag && arc_cache->l1[i]->valid == true)
        {
            // t1 is of size p, and t2 is of size cache_size - p
            // block is in t1
            if(i < arc_cache->p)
            {
                return arc_cache->l1[i];
            }
            // block is in b1
            else
            {
                arc_cache->p += 1;
                shiftL2Down1Index(arc_cache);
                arc_cache->l2[0] = arc_cache->l1[i];
                // Everything after the extracted block moves up
                for (j = cache_size; j > i; j--)
                {
                    arc_cache->l1[j] = arc_cache->l1[j-1];
                }
                return NULL;
            }
        }
        else if(tag == arc_cache->l2[i]->tag && arc_cache->l2[i]->valid == true)
        {
            // block is in t2
            if (i < (cache_size - arc_cache->p))
            {
                return arc_cache->l2[i];
            }
            // block is in b2
            else
            {
                arc_cache->p -= 1;
                shiftL2Down1Index(arc_cache);
                arc_cache->l2[0] = arc_cache->l2[i];
                // Everything after the extracted block moves up
                for (j = cache_size; j > i; j--)
                {
                    arc_cache->l2[j] = arc_cache->l2[j-1];
                }
                return NULL;
            }
        }
    }

    return NULL;
}
bool insertBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time)
{
    uint64_t blk_aligned_addr = blkAlign(req->load_or_store_addr, arc_cache->blk_mask);
    Cache_Block *victim = NULL;
    bool wb_required = arc(arc_cache, blk_aligned_addr, &victim);
    assert(victim != NULL);
    uint64_t tag = req->load_or_store_addr >> arc_cache->tag_shift;
    victim->tag = tag;
    victim->valid = true;
    victim->when_touched = access_time;
    ++victim->frequency;
    if (req->req_type == STORE)
    {
        victim->dirty = true;
    }

    return wb_required;
}

bool accessBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time)
{
    bool hit = false;

    uint64_t blk_aligned_addr = blkAlign(req->load_or_store_addr, arc_cache->blk_mask);

    Cache_Block *blk = findBlockARC(arc_cache, blk_aligned_addr);
   
    if (blk != NULL) 
    {
        hit = true;
        // Update access time	
        blk->when_touched = access_time;
        // Increment frequency counter
        ++blk->frequency;

        if (req->req_type == STORE)
        {
            blk->dirty = true;
        }
    }

    return hit;
}

// Helper Functions
inline uint64_t blkAlign(uint64_t addr, uint64_t mask)
{
    return addr & ~mask;
}


bool lru(Cache *cache, uint64_t addr, Cache_Block **victim_blk)
{
    uint64_t set_idx = (addr >> cache->set_shift) & cache->set_mask;
    Cache_Block **ways = cache->sets[set_idx].ways;
    int i;
    for (i = 0; i < cache->num_ways; i++)
    {
        if (ways[i]->valid == false)
        {
            *victim_blk = ways[i];
            return false; // No need to write-back
        }
    }

    // Step two, if there is no invalid block. Locate the LRU block
    Cache_Block *victim = ways[0];
    for (i = 1; i < cache->num_ways; i++)
    {
        if (ways[i]->when_touched < victim->when_touched)
        {
            victim = ways[i];
        }
    }

    // Step three, need to write-back the victim block
    // *wb_addr = (victim->tag << cache->tag_shift) | (victim->set << cache->set_shift);
//    uint64_t ori_addr = (victim->tag << cache->tag_shift) | (victim->set << cache->set_shift);
//    printf("Evicted: %"PRIu64"\n", ori_addr);

    // Step three, invalidate victim
    victim->tag = UINTMAX_MAX;
    victim->valid = false;
    victim->dirty = false;
    victim->frequency = 0;
    victim->when_touched = 0;

    *victim_blk = victim;

    return true; // Need to write-back
}

bool lfu(Cache *cache, uint64_t addr, Cache_Block **victim_blk, uint64_t *wb_addr)
{
    uint64_t set_idx = (addr >> cache->set_shift) & cache->set_mask;
    //    printf("Set: %"PRIu64"\n", set_idx);
    Cache_Block **ways = cache->sets[set_idx].ways;

    // Step one, try to find an invalid block.
    int i;
    for (i = 0; i < cache->num_ways; i++)
    {
        if (ways[i]->valid == false)
        {
            *victim_blk = ways[i];
            return false; // No need to write-back
        }
    }

    // Step two, if there is no invalid block. Locate the LFU block
    Cache_Block *victim = ways[0];
    for (i = 1; i < cache->num_ways; i++)
    {
        if (ways[i]->frequency < victim->frequency)
        {
            victim = ways[i];
        }
    }

    // Step three, need to write-back the victim block
    *wb_addr = (victim->tag << cache->tag_shift) | (victim->set << cache->set_shift);
//    uint64_t ori_addr = (victim->tag << cache->tag_shift) | (victim->set << cache->set_shift);
//    printf("Evicted: %"PRIu64"\n", ori_addr); 

    // Step three, invalidate victim
    victim->tag = UINTMAX_MAX;
    victim->valid = false;
    victim->dirty = false;
    victim->frequency = 0;
    victim->when_touched = 0;

    *victim_blk = victim;

    return true; // Need to write-back
}

bool arc(ARCCache *arc_cache, uint64_t addr, Cache_Block **victim_blk)
{
    int i;
    for (i = 0; i < cache_size; i++)
    {
        if (arc_cache->l1[i]->valid == false)
        {
            *victim_blk = arc_cache->l1[i];
            return false; // No need to write-back
        }
    }
    Cache_Block *victim;
    shiftL1Down1Index(arc_cache);
    victim = arc_cache->l1[0];

    victim->tag = UINTMAX_MAX;
    victim->valid = false;
    victim->dirty = false;
    victim->frequency = 0;
    victim->when_touched = 0;
    *victim_blk = victim;
    return true; // Need to write-back
}
