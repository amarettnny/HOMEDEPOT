#include "cache_sim.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
using namespace std;
/*
 * Check whether an input integer is power of 2
 *
 * Parameters:
 *  x: the input integer to check
 *
 * Returns:
 *  true if x is a power of 2, false if not
 */
bool is_valid_num(int x) { return x > 0 && (x & (x - 1)) == 0; }

/*
 * Check whether the input cache size(sets), set sizes(blocks), and store
 * procedures is valid, print corresponding error message for invalid arguments
 *
 * Parameters:
 *  sets: number of sets in the cache
 *  blocks: number of blocks in a set
 *  write_alloc: true is write-allocate, false if no-write-allocate
 *  write_back: true if write-back, false if write-through
 *
 * Returns:
 *  true if the arguments combination is valid, false if not
 */
bool is_valid_argument(int sets, int blocks, int bytes, bool write_alloc,
                       bool write_back) {
  bool is_valid = true;
  if (!is_valid_num(sets) || !is_valid_num(blocks) || !is_valid_num(bytes) ||
      bytes < 4) {
    is_valid = false;
    fprintf(stderr, "Invalid sets/blocks number.\n");
  }
  if (!write_alloc && write_back) {
    is_valid = false;
    fprintf(stderr, "write-back and no-write-allocate were both specified.\n");
  }
  return is_valid;
}

/*
 * Initializer for cache, initialize an empty cache based on the input arguments
 *
 * Parameters:
 *   num_sets: number of sets in cache
 *   blocks: number of blocks in set
 *   bytes: number of bytes in block
 *   write_alloc: true if write-allocate, false is no-write-allocate
 *   write_back: true if write-back, false if write-through
 *   evic_policy: lru or fifo based on the input
 */
Cache::Cache(int num_sets, int blocks, int bytes, bool write_alloc,
             bool write_back, std::string evic_policy)
    : num_sets(num_sets), blocks(blocks), bytes(bytes),
      write_alloc(write_alloc), write_back(write_back),
      evic_policy(evic_policy), loads(0), stores(0), load_hits(0),
      load_misses(0), store_hits(0), store_misses(0), total_cycles(0) {
  sets.resize(num_sets);
  for (Set &set : sets) {
    set.blocks.resize(blocks);
    int idx = 0;
    for (Block &block : set.blocks) {
      block.valid = false;
      block.dirty = false;
      block.tag = 0;
      block.load_ts = idx;
      block.access_ts =
          idx; // ts are set from 0, 1, ... to block-1 at the beginning
      idx++;
    }
  }
}

/*
 * store the memory to cache from input mem_addr, update the cache and its stats
 * accordingly
 *
 * Parameters:
 *  mem_addr: input memopry address
 */
void Cache::loading(unsigned int mem_addr) {
  loads += 1;
  total_cycles += 1; // each load has a load from cache
  unsigned int index_len = log2(num_sets);
  unsigned int off_len = log2(bytes);
  int index_cache = (mem_addr >> off_len) & (num_sets - 1);
  Set &curr_set = sets[index_cache];

  // Calculate the tag by shifting out index and offset bits
  unsigned int blk_cnt = mem_addr >> (index_len + off_len);

  if (evic_policy == "lru") {
    loading_lru(curr_set, blk_cnt);
  } else if (evic_policy == "fifo") {
    // loading_fifo(curr_set, blk_cnt);
  }
}

void Cache::loading_lru(Set &curr_set, unsigned int blk_cnt) {
  bool hit = false;
  int index_blk = -1;
  int blk_access = -1;

  // Search the set for a block with matching tag
  for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
    Block &block = curr_set.blocks[i];
    if (block.tag == blk_cnt && block.valid) {
      hit = true;
      blk_access = block.access_ts;
      block.access_ts = 0; // if hit, update the lru time
      index_blk = i;
      break;
    }
  }

  if (hit == true) {
    load_hits += 1;
    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      if ((int)curr_set.blocks[i].access_ts < blk_access &&
          (int)i != index_blk) {
        curr_set.blocks[i].access_ts += 1;
        // if hit, increase those lru time that is smaller than the hitted by
        // 1, maintain the visit order
      }
    }
  } else { // If not hit
    // Replace LRU in the set
    load_misses += 1;
    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      if ((int)curr_set.blocks[i].access_ts == blocks - 1) {
        // block with largest order is the least recent visited, replace this
        // block, reset visit order to 0
        curr_set.blocks[i].access_ts = 0;
        curr_set.blocks[i].tag = blk_cnt;
        curr_set.blocks[i].valid = true;
        total_cycles += 100 * (bytes / 4);
        if (curr_set.blocks[i].dirty == true) {
          total_cycles += 100 * (bytes / 4);
          curr_set.blocks[i].dirty = false;
        }
      } else {
        curr_set.blocks[i].access_ts +=
            1; // increase other lru to maintain the order
      }
    }
  }
}

/*
 * Store the memory to cache from input mem_addr, update the cache and its stats
 * accordingly.
 *
 * Parameters:
 *  mem_addr: input memopry address
 */
void Cache::storing(unsigned int mem_addr) {
  stores += 1;
  total_cycles += 1; // Each store cost a basic total cycle

  unsigned int index_len = log2(num_sets);
  unsigned int off_len = log2(bytes);
  int index_cache = (mem_addr >> off_len) & (num_sets - 1); // Target set
  Set &curr_set = sets[index_cache];

  // Calculate the tag by shifting out index and offset bits
  unsigned int blk_cnt = mem_addr >> (index_len + off_len);

  if (evic_policy == "lru") {
    storing_lru(curr_set, blk_cnt);
  } else {
    storing_fifo(curr_set, blk_cnt);
  }
}

/*
 * Helper function for storing when using LRU eviction policy.
 *
 * Parameters:
 *  curr_set: the cache set in which the store operation is being performed
 *  blk_cnt: the tag for identifying the cache block
 */
void Cache::storing_lru(Set &curr_set, unsigned int blk_cnt) {
  bool hit = false;
  int index_blk = -1;
  int blk_access = -1;

  // Search the set for a block with matching tag
  for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
    Block &block = curr_set.blocks[i];
    if (block.tag == blk_cnt && block.valid) {
      hit = true;
      blk_access = block.access_ts;
      block.access_ts =
          0; // lru policy updates the order on hit --> set most recent to 0
      index_blk = i;
      break;
    }
  }

  if (hit == true) {
    store_hits += 1;
    // Add access timestamp for remaining blocks that are accessed before the
    // target
    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      if ((int)curr_set.blocks[i].access_ts < blk_access &&
          (int)i != index_blk) {
        curr_set.blocks[i].access_ts += 1;
      }
    }
    if (write_back) {
      // Mark written (dirty) when write back
      curr_set.blocks[index_blk].dirty = true;
    } else {
      total_cycles += 100; // write-through cost
    }
  } else {
    store_misses += 1;
    if (write_alloc) {
      // When miss, load the block first
      total_cycles += 100 * (bytes / 4);
      for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
        // look for block with highest access_ts
        if ((int)curr_set.blocks[i].access_ts == blocks - 1) {
          // Evict the least used block
          if (curr_set.blocks[i].dirty == true) {
            total_cycles += 100 * (bytes / 4);
            curr_set.blocks[i].dirty = false;
          }
          curr_set.blocks[i].tag = blk_cnt;
          curr_set.blocks[i].valid = true;
          curr_set.blocks[i].access_ts = 0; // reset timestamp
          if (write_back) {
            curr_set.blocks[i].dirty = true;
          } else {
            total_cycles += 100; // write-through cost
          }
        } else {
          curr_set.blocks[i].access_ts += 1;
        }
      }
    } else {
      // If no write-allocate --> directly write to memory
      total_cycles += 100;
    }
  }
}

/*
 * Helper function for storing when using FIFO eviction policy.
 *
 * Parameters:
 *  curr_set: the cache set in which the store operation is being performed
 *  blk_cnt: the tag for identifying the cache block
 */
void Cache::storing_fifo(Set &curr_set, unsigned int blk_cnt) {
  bool hit = false;
  int index_blk = -1;
  // Search the set for a block with matching tag
  for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
    Block &block = curr_set.blocks[i];
    if (block.tag == blk_cnt &&
        block.valid) { // fifo does not update order on hit
      hit = true;
      index_blk = i;
      break;
    }
  }
  if (hit) {
    store_hits += 1;
    if (write_back) {
      curr_set.blocks[index_blk].dirty = true;
    } else {
      total_cycles += 100; // write-through cost on hit
    }
  } else {
    store_misses += 1;
    storing_miss_fifo(curr_set, blk_cnt);
  }
}

/*
 * Helper function for handling storing when hit misses and FIFO eviction
 * policy is being used. 
 *
 * Parameters:
 *  curr_set: the cache set in which the store operation is being performed
 *  blk_cnt: the tag for identifying the cache block
 */
void Cache::storing_miss_fifo(Set &curr_set, unsigned int blk_cnt) {
  if (write_alloc) {
    // When miss, load the block first, add the penalty
    total_cycles += 100 * (bytes / 4);
    // Find the fifo candidate -> the block with the smallest load_ts
    int fifo_index = 0;
    unsigned int min_ts = curr_set.blocks[0].load_ts;
    for (unsigned long int i = 1; i < curr_set.blocks.size(); i++) {
      if (curr_set.blocks[i].load_ts < min_ts) {
        min_ts = curr_set.blocks[i].load_ts;
        fifo_index = i;
      }
    }

    // If dirty, write back before update
    if (curr_set.blocks[fifo_index].dirty) {
      total_cycles += 100 * (bytes / 4);
      curr_set.blocks[fifo_index].dirty = false;
    }
    // Update candidate with the new block
    curr_set.blocks[fifo_index].tag = blk_cnt;
    curr_set.blocks[fifo_index].valid = true;
    // Set new timestamp -> set to maximum load_ts + 1 -> became last in
    unsigned int max_ts = 0;
    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      if (curr_set.blocks[i].load_ts > max_ts)
        max_ts = curr_set.blocks[i].load_ts;
    }
    curr_set.blocks[fifo_index].load_ts = max_ts + 1;

    if (write_back) {
      curr_set.blocks[fifo_index].dirty = true;
    } else {
      total_cycles += 100; // write-through cost
    }
  } else {
    // If no write-allocate --> directly write to memory
    total_cycles += 100;
  }
}

/*
 * Print out the statistics of the simulation
 */
void Cache::print_stats() {
  std::cout << "Total loads: " << loads << endl;
  std::cout << "Total stores: " << stores << endl;
  std::cout << "Load hits: " << load_hits << endl;
  std::cout << "Load misses: " << load_misses << endl;
  std::cout << "Store hits: " << store_hits << endl;
  std::cout << "Store misses: " << store_misses << endl;
  std::cout << "Total cycles: " << total_cycles << endl;
}
