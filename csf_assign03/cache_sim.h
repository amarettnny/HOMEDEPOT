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

struct Cache{
    std::vector<Set> sets;
    int num_sets;
    int blocks;
    int bytes;
    bool write_alloc;
    bool write_back;
    std::string evic_policy;

    // Stats
    int loads;
    int stores;
    int load_hits;
    int load_misses;
    int store_hits;
    int store_misses;
    int total_cycles;

    Cache(int num_sets, int blocks, int bytes, bool write_alloc, bool write_back, std::string evic_policy);

    void loading();
    void storing();

    void print_stats();
};

bool is_valid_argument(int sets, int blocks, bool write_alloc, bool write_back);


#endif