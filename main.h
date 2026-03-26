/* main.h */
#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <cstdint>
#include "cache.h"

// The cache config shape
struct Config {
    uint64_t num_sets;
    uint64_t block_per_set;
    uint64_t block_size;
    bool write_allocate;
    bool write_through;
    bool lru;
};

// The cache operation shape
struct Operation {
    bool is_load; // true for load, false for store
    uint32_t address;
};

// The stats result shape
struct Stats {
    uint64_t total_loads; 
    uint64_t total_stores;
    uint64_t load_hits;
    uint64_t load_misses;
    uint64_t store_hits;
    uint64_t store_misses;
    long long total_cycles;
};

Config validate_and_parse_config(int argc, char* argv[]);
std::vector<Operation> read_operations();
Address decompose_address(uint32_t address, Config config);
void load_address(Address addr, Cache& cache, Config config, Stats& stats, uint32_t global_time);
void store_address(Address addr, Cache& cache, Config config, Stats& stats, uint32_t global_time);

#endif // MAIN_H