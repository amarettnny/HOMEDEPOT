#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "cache_sim.h"

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
    fprintf(stderr, "Invalid eviction method");
    return 1;
  }

  if (!is_valid_argument(sets, blocks, write_alloc, write_back)){
    fprintf(stderr, "Invalid cache parameters");
    return 1;
  }

  // a while process to read all text input
  // if 'l' --> cache_write()
  // if 's' --> cache_read()

  



  return 0;
}
