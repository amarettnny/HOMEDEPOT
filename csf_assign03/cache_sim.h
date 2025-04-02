#ifndef CACHE_SIM_H
#define CACHE_SIM_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct Block {
  bool valid, dirty;
  uint32_t tag;
  uint32_t load_ts, access_ts;
};

struct Set {
  std::vector<Block> blocks;
};

struct Cache {
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
  /*
   * Initializer for cache, initialize an empty cache based on the input
   * arguments
   *
   * Parameters:
   *   num_sets: number of sets in cache
   *   blocks: number of blocks in set
   *   bytes: number of bytes in block
   *   write_alloc: true if write-allocate, false is no-write-allocate
   *   write_back: true if write-back, false if write-through
   *   evic_policy: lru or fifo based on the input
   */
  Cache(int num_sets, int blocks, int bytes, bool write_alloc, bool write_back,
        std::string evic_policy);

  /*
   * load the memory to cache from input mem_addr, update cache and its stats
   * accordingly
   *
   * Parameters:
   *  mem_addr: input memory address
   */
  void loading(unsigned int mem_addr);

  void loading_lru(Set &curr_set, unsigned int blk_cnt);
  // void loading_fifo(Set &curr_set, unsigned int blk_cnt);

  /*
   * store the memory to cache from input mem_addr, update the cache and its
   * stats accordingly
   *
   * Parameters:
   *  mem_addr: input memopry address
   */
  void storing(unsigned int mem_addr);

  /*
   * Helper function for storing when using LRU eviction policy.
   *
   * Parameters:
   *  curr_set: the cache set in which the store operation is being performed
   *  blk_cnt: the tag for identifying the cache block
   */
  void storing_lru(Set &curr_set, unsigned int blk_cnt);

  /*
   * Helper function for storing when using FIFO eviction policy.
   *
   * Parameters:
   *  curr_set: the cache set in which the store operation is being performed
   *  blk_cnt: the tag for identifying the cache block
   */
  void storing_fifo(Set &curr_set, unsigned int blk_cnt);

  /*
   * Helper function for handling storing when hit misses and FIFO eviction
   * policy is being used.
   *
   * Parameters:
   *  curr_set: the cache set in which the store operation is being performed
   *  blk_cnt: the tag for identifying the cache block
   */
  void storing_miss_fifo(Set &curr_set, unsigned int blk_cnt);

  /*
   * Print out the statistics of the simulation
   */
  void print_stats();
};

/*
 * Check whether an input integer is power of 2
 *
 * Parameters:
 *  x: the input integer to check
 *
 * Returns:
 *  true if x is a power of 2, false if not
 */
bool is_valid_num(int x);

/*
 * Check whether the input cache size(sets), set sizes(blocks), and store
 * procedures is valid print corresponding error message for invalid arguments
 *
 * Parameters:
 *  sets: number of sets in the cache
 *  blocks: number of blocks in a set
 *  bytes: number of bytes in a block
 *  write_alloc: true is write-allocate, false if no-write-allocate
 *  write_back: true if write-back, false if write-through
 *
 * Returns:
 *  true if the arguments combination is valid, false if not
 */
bool is_valid_argument(int sets, int blocks, int bytes, bool write_alloc,
                       bool write_back);

#endif
