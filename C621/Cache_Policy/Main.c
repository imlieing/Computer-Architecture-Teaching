#include "Trace.h"
#include "Cache.h"

extern TraceParser *initTraceParser(const char * mem_file);
extern bool getRequest(TraceParser *mem_trace);

extern Cache* initCache();
extern bool accessBlock(Cache *cache, Request *req, uint64_t access_time);
extern bool insertBlock(Cache *cache, Request *req, uint64_t access_time, uint64_t *wb_addr);

extern Cache* initARCLists();
extern bool accessBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time);
extern bool insertBlockARC(ARCCache *arc_cache, Request *req, uint64_t access_time);

// #define ARC


int main(int argc, const char *argv[])
{	

    if (argc != 2)
    {
        printf("Usage: %s %s\n", argv[0], "<mem-file>");

        return 0;
    }

    // Initialize a CPU trace parser
    TraceParser *mem_trace = initTraceParser(argv[1]);
    Cache *cache = initCache();

    // Initialize a Cache
    #ifdef ARC
        ARCCache *arc_cache = initARCCache();
    #endif
    
    // Running the trace
    uint64_t num_of_reqs = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t num_evicts = 0;
    uint64_t cycles = 0;

    while (getRequest(mem_trace))
    {
        // Step one, accessBlock()
        #ifdef ARC
            if(accessBlockARC(arc_cache, mem_trace->cur_req, cycles))
            {
                hits++;
            }
            else
            {
                misses++;
                if (insertBlockARC(arc_cache, mem_trace->cur_req, cycles))
                {
                    num_evicts++;
                }
            }
        #else

            #ifdef STRIDE
                int i = 0;
                for (int i = 0; i < cache->cache_size; i++)
                {
                    if (cache->pc_list[i] == mem_trace->cur_req->PC)
                    {
                        cache->current_stride = abs(mem_trace->cur_req->load_or_store_addr - cache->address_list[i]);
                        cache->address_list[i] = mem_trace->cur_req->load_or_store_addr;
                        break;
                    }
                    else if (cache->pc_list[i] == 0)
                    {
                        cache->pc_list[i] = mem_trace->cur_req->PC;
                        cache->address_list[i] = mem_trace->cur_req->load_or_store_addr;
                        break;
                    }
                }
            #endif

            if (accessBlock(cache, mem_trace->cur_req, cycles))
            {
                // Cache hit
                hits++;
            }
            else
            {
                // Cache miss!
                misses++;
                // Step two, insertBlock()
    //            printf("Inserting: %"PRIu64"\n", mem_trace->cur_req->load_or_store_addr);
                uint64_t wb_addr;
                int i = 0;
                int n_line_prefetch = 10;
                int block_size = 64;
                #ifdef PREFETCH
                    for (i = 0; i <= n_line_prefetch; i++)
                    {
                        mem_trace->cur_req->load_or_store_addr = mem_trace->cur_req->load_or_store_addr + i * block_size;
                        if (insertBlock(cache, mem_trace->cur_req, cycles, &wb_addr))
                        {
                            num_evicts++;
            //                printf("Evicted: %"PRIu64"\n", wb_addr);
                        }
                    }
                
                #elif defined STRIDE
                    if (insertBlock(cache, mem_trace->cur_req, cycles, &wb_addr))
                    {
                        num_evicts++;
                    }
                    mem_trace->cur_req->load_or_store_addr = mem_trace->cur_req->load_or_store_addr + cache->current_stride;
                    if (insertBlock(cache, mem_trace->cur_req, cycles, &wb_addr))
                    {
                        num_evicts++;
                    }
                    
                #else
                    if (insertBlock(cache, mem_trace->cur_req, cycles, &wb_addr))
                    {
                        num_evicts++;
        //                printf("Evicted: %"PRIu64"\n", wb_addr);
                    }
                #endif
            }
        #endif

        ++num_of_reqs;
        ++cycles;
    }

    double hit_rate = (double)hits / ((double)hits + (double)misses);
    printf("Hit rate: %lf%%\n", hit_rate * 100);
}
