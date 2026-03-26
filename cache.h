/* cache.h */
#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <cstdint>

struct Address { 
    uint32_t tag;
    uint32_t index;
    uint32_t offset;
};

struct Slot {
    uint32_t tag;
    bool valid, dirty;
    uint32_t load_ts,
    access_ts;
};

struct Set {
    std::vector<Slot> slots;
};

struct Cache {
    std::vector<Set> sets;
};

#endif // CACHE_H