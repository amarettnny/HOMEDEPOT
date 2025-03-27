#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ratio>
#include <string>
#include "cache_sim.h"

using namespace std;

bool is_valid_num(int x) {
    return x > 0 && (x & (x - 1)) == 0;
}

bool is_valid_argument(int sets, int blocks, bool write_alloc, bool write_back) {
    bool is_valid;
    if (!is_valid_num(sets) || !is_valid_num(blocks)){
        is_valid = false;
        fprintf(stderr, "Invalid sets/blocks number. \n");

    }
    if (!write_alloc && write_back){
        is_valid = false;
        fprintf(stderr, "write-back and no-write-allocate were both specified. \n");
    }
    return is_valid;
}

