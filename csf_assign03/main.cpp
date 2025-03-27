#include "cache_sim.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc != 7) {
    fprintf(stderr, "Missing input argument for cache simulation.\n");
    return 1; // nonzero exit code
  }

  // Read command-line argument
  int sets = std::stoi(argv[1]);
  int blocks = std::stoi(argv[2]);
  int bytes = std::stoi(argv[3]);

  bool write_alloc, write_back;
  std::string evic_policy = argv[6];

  // Check if string inputs are valid
  if (strcmp(argv[4], "write-allocate") == 0) {
    write_alloc = true;
  } else if (strcmp(argv[4], "no-write-allocate") == 0) {
    write_alloc = false;
  } else {
    fprintf(stderr, "Invalid write policy.\n");
    return 1;
  }

  if (strcmp(argv[5], "write-back") == 0) {
    write_back = true;
  } else if (strcmp(argv[5], "write-through") == 0) {
    write_back = false;
  } else {
    fprintf(stderr, "Invalid write method.\n");
    return 1;
  }

  if (evic_policy != "lru" && evic_policy != "fifo") {
    fprintf(stderr, "Invalid eviction method.\n");
    return 1;
  }

  // Check invalid cache arguments
  if (!is_valid_argument(sets, blocks, bytes, write_alloc, write_back)) {
    return 1;
  }

  // Initializing Cache_sim to pass on parameters
  Cache cache_sim(sets, blocks, bytes, write_alloc, write_back, evic_policy);

  // Start reading from the trace, line by line
  std::string line;
  while (std::getline(std::cin, line)) {
    char op;
    uint64_t addr;
    int data;  // We're ignoring this data for this assignment
    sscanf(line.c_str(), "%c %lx %d", &op, &addr, &data);

    if (op == 'l'){ // Handle loading
      cache_sim.loading(addr);
    } else if (op == 's'){ // Handle storing
      cache_sim.storing(addr);
    } else {
      fprintf(stderr, "Invalid operation to Cache.\n");
    }
  }

  // Print out the stats
  cache_sim.print_stats();

  return 0;
}
