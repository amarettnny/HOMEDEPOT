#ifndef CACHE_SIM_H
#define CACHE_SIM_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct Block{
    bool valid, dirty;
    uint32_t tag;
    uint32_t load_ts, access_ts;
};

struct Set{
    std::vector<Block> blocks;
};

struct Cache_sim{
    std::vector<Set> sets;
    int num_sets;
    int blocks_per_set;
    int bytes_per_block;
    bool write_allocate;
    bool write_back;
    bool is_lru;

    // Stats
    int loads;
    int stores;
    int load_hits;
    int load_misses;
    int store_hits;
    int store_misses;
};

bool is_valid_argument(int sets, int blocks, bool write_alloc, bool write_back);


#endif