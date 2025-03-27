#include "cache_sim.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ratio>
#include <string>
using namespace std;

bool is_valid_num(int x) { return x > 0 && (x & (x - 1)) == 0; }

bool is_valid_argument(int sets, int blocks, bool write_alloc,
                       bool write_back) {
  bool is_valid = true;
  if (!is_valid_num(sets) || !is_valid_num(blocks)) {
    is_valid = false;
    fprintf(stderr, "Invalid sets/blocks number. \n");
  }
  if (!write_alloc && write_back) {
    is_valid = false;
    fprintf(stderr, "write-back and no-write-allocate were both specified. \n");
  }
  return is_valid;
}

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
      block.access_ts = idx;
    }
  }
}

void Cache::loading(unsigned int mem_addr) {
  loads += 1;
  total_cycles += 1;
  unsigned int index_len = log2(num_sets);
  unsigned int off_len = log2(bytes);
  int index_cache = (mem_addr >> off_len) & (num_sets - 1);
  Set &curr_set = sets[index_cache];
  unsigned int blk_cnt = mem_addr >> (index_len + off_len);
  if (evic_policy == "lru") {
    bool hit = false;
    int index_blk = -1;
    int blk_access = -1;
    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      Block &block = curr_set.blocks[i];
      if (block.tag == blk_cnt && block.valid) {
        hit = true;
        blk_access = block.access_ts;
        block.access_ts = 0;
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
        }
      }
    } else {
      load_misses += 1;
      for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
        if ((int)curr_set.blocks[i].access_ts == blocks - 1) {
          curr_set.blocks[i].access_ts = 0;
          curr_set.blocks[i].tag = blk_cnt;
          curr_set.blocks[i].valid = true;
          total_cycles += 100 * (bytes / 4);
          if (curr_set.blocks[i].dirty == true) {
            total_cycles += 100 * (bytes / 4);
            curr_set.blocks[i].dirty = false;
          }
        } else {
          curr_set.blocks[i].access_ts += 1;
        }
      }
    }
  }
}

void Cache::storing(unsigned int mem_addr) {
  stores += 1;
  total_cycles += 1;

  unsigned int index_len = log2(num_sets);
  unsigned int off_len = log2(bytes);
  int index_cache = (mem_addr >> off_len) & (num_sets - 1);
  Set &curr_set = sets[index_cache];
  unsigned int blk_cnt = mem_addr >> (index_len + off_len);

  if (evic_policy == "lru") {
    bool hit = false;
    int index_blk = -1;
    int blk_access = -1;

    for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
      Block &block = curr_set.blocks[i];
      if (block.tag == blk_cnt && block.valid) {
        hit = true;
        blk_access = block.access_ts;
        block.access_ts = 0;
        index_blk = i;
        break;
      }
    }

    if (hit == true) {
      store_hits += 1;
      for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
        if ((int)curr_set.blocks[i].access_ts < blk_access &&
            (int)i != index_blk) {
          curr_set.blocks[i].access_ts += 1;
        }
      }

      if (write_back) {
        curr_set.blocks[index_blk].dirty = true;
      } else {
        total_cycles += 100 * (bytes / 4); // write-through cost
      }
    } else {
      store_misses += 1;

      if (write_alloc) {
        // Do a load first (miss)
        total_cycles += 100 * (bytes / 4);
        for (unsigned long int i = 0; i < curr_set.blocks.size(); i++) {
          if ((int)curr_set.blocks[i].access_ts == blocks - 1) {
            // Evict if needed
            if (curr_set.blocks[i].dirty == true) {
              total_cycles += 100 * (bytes / 4);
              curr_set.blocks[i].dirty = false;
            }
            curr_set.blocks[i].tag = blk_cnt;
            curr_set.blocks[i].valid = true;
            curr_set.blocks[i].access_ts = 0;
            if (write_back) {
              curr_set.blocks[i].dirty = true;
            } else {
              total_cycles += 100 * (bytes / 4); // write-through
            }
          } else {
            curr_set.blocks[i].access_ts += 1;
          }
        }
      } else {
        // No write-allocate: directly write to memory
        total_cycles += 100 * (bytes / 4);
      }
    }
  }
}

void Cache::print_stats() {
  std::cout << "Total loads: " << loads << endl;
  std::cout << "Total stores: " << stores << endl;
  std::cout << "Load hits: " << load_hits << endl;
  std::cout << "Load misses: " << load_misses << endl;
  std::cout << "Store hits: " << store_hits << endl;
  std::cout << "Store misses: " << store_misses << endl;
  std::cout << "Total cycles: " << total_cycles << endl;
}
