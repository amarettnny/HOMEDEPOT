#include "cache_sim.h"
#include "cache_sim.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  // Read command-line argument
  if (argc != 7) {
    fprintf(stderr, "Missing input argument for cache simulation. \n");
    return 1; // nonzero exit code
  }

  int sets = std::stoi(argv[1]);
  int blocks = std::stoi(argv[2]);
  int bytes = std::stoi(argv[3]);

  bool write_alloc, write_back;
  std::string evic_policy = argv[6];

  if (strcmp(argv[4], "write-allocate")) {
    write_alloc = true;
  } else if (strcmp(argv[4], "no-write-allocate")) {
    write_alloc = false;
  } else {
    fprintf(stderr, "Invalid write policy.\n");
    return 1;
  }

  if (strcmp(argv[5], "write-back")) {
    write_back = true;
  } else if (strcmp(argv[5], "write-through")) {
    write_back = false;
  } else {
    fprintf(stderr, "Invalid write method.\n");
    return 1;
  }

  if (evic_policy != "lru" && evic_policy != "fifo") {
    fprintf(stderr, "Invalid eviction method. \n");
    return 1;
  }

  //if (!is_valid_argument(sets, blocks, write_alloc, write_back)) {
  //  return 1;
  //}

  Cache cache_sim(sets, blocks, bytes, write_alloc, write_back, evic_policy);

  std::string line;
  while (std::getline(std::cin, line)) {
    char op;
    uint64_t addr;
    int data;     // We're ignoreing this data for this assignment
    sscanf(line.c_str(), "%c %lx %d", &op, &addr, &data);

    if (op == 'l'){
	cache_sim.loading(addr);
    } else if (op == 's'){
      //cache_sim.storing();
    } else{
      fprintf(stderr, "Invalid operation to Cache. \n");
    }
  }

  cache_sim.print_stats();

  return 0;
}
