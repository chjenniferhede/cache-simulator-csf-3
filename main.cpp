#include <iostream>
#include <string> 
#include <vector>

#include "main.h"
#include "cache.h"

int main(int argc, char* argv[]) {
    // Check, 6 parameters needed
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <num_sets> <block_per_set> <block_size> <write_allocate> <write_back> <lru>" << std::endl;
        return 1;
    }
    // Validate and collect config from params, program abort here if invalid
    Config config = validate_and_parse_config(argc, argv);

    // Config are valid, we parse the operations from stdin
    std::vector<Operation> operations = read_operations();

    // We have all informations, create the empty cache
    Cache cache;
    cache.sets.resize(config.num_sets);
    for (auto& set : cache.sets) {
        set.slots.resize(config.block_per_set);
        // This sets tag = 0, valid = false, dirty = false, load_ts = 0, access_ts = 0 for all slots
    }

    // Initialize the stats
    Stats stats = {0, 0, 0, 0, 0, 0, 0};

    // Run through the operations
    uint32_t global_time = 0; 
    for (const auto& op : operations) {
        Address addr = decompose_address(op.address, config);

        // Route load vs. store
        if (op.is_load) {
            stats.total_loads++;
            load_address(addr, cache, config, stats, global_time);
        } else {
            stats.total_stores++;
            store_address(addr, cache, config, stats, global_time);
        }

        global_time++; 
    }

    // Output the stats
    std::cout << "Total loads: " << stats.total_loads << std::endl;
    std::cout << "Total stores: " << stats.total_stores << std::endl;
    std::cout << "Load hits: " << stats.load_hits << std::endl;
    std::cout << "Load misses: " << stats.load_misses << std::endl;
    std::cout << "Store hits: " << stats.store_hits << std::endl;
    std::cout << "Store misses: " << stats.store_misses << std::endl;
    std::cout << "Total cycles: " << stats.total_cycles << std::endl;
}

// Helper function to validate and parse the config parameters
Config validate_and_parse_config(int argc, char* argv[]) {
    Config config;

    // Parse the params and put in the Config struct, then report parsing errors. 
    try {
        config.num_sets = std::stoi(argv[1]);
        config.block_per_set = std::stoi(argv[2]);
        config.block_size = std::stoi(argv[3]);

        // For the 3 bool params, we need to check the content
        if (std::string(argv[4]) != "write-allocate" && std::string(argv[4]) != "no-write-allocate") {
            throw std::invalid_argument("Expected 'write-allocate' or 'no-write-allocate' for param 4.");
        }
        config.write_allocate = (argv[4] == std::string("write-allocate"));
        if (std::string(argv[5]) != "write-through" && std::string(argv[5]) != "write-back") {
            throw std::invalid_argument("Expected 'write-through' or 'write-back' for param 5.");
        }
        config.write_through = (argv[5] == std::string("write-through"));
        if (std::string(argv[6]) != "lru" && std::string(argv[6]) != "fifo") {
            throw std::invalid_argument("Expected 'lru' or 'fifo' for param 6.");
        }
        config.lru = (argv[6] == std::string("lru"));

    // Report error if any parsing error occurs.
    } catch (const std::exception& e) {
        std::cerr << "Error parsing parameters: " << e.what() << std::endl;
        exit(1);
    }

    // Now find logical errors
    // Check if num_sets, block_per_set, block_size are positive
    if (config.num_sets <= 0 || config.block_per_set <= 0 || config.block_size <= 0) {
        std::cerr << "Error: num_sets, block_per_set, and block_size must be positive integers." << std::endl;
        exit(1);
    }
    // Check if they are power of two 
    if ((config.num_sets & (config.num_sets - 1)) != 0 || (config.block_per_set & (config.block_per_set - 1)) != 0 || (config.block_size & (config.block_size - 1)) != 0) {
        std::cerr << "Error: num_sets, block_per_set, and block_size must be powers of two." << std::endl;
        exit(1);
    }
    // Block size must be at least 4 bytes (32 bits)
    if (config.block_size < 4) {
        std::cerr << "Error: block_size must be at least 4 bytes." << std::endl;
        exit(1);
    }
    // Cnanot be write-back and no-write-allocate at the same time
    if (!config.write_through && !config.write_allocate) {
        std::cerr << "Error: Cache cannot be both write-back and no-write-allocate." << std::endl;
        exit(1);
    }

    return config;
}

// Helper function to read operations from stdin
std::vector<Operation> read_operations() {
    std::vector<Operation> operations;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue; // Skip empty lines

        char op_type;
        uint32_t address;
        int ignore;

        // Parse the line <type> <address> <ignore>
        if (std::sscanf(line.c_str(), "%c %x %d", &op_type, &address, &ignore) != 3) {
            std::cerr << "Error: Failed to parse operation line." << std::endl;
            exit(1);
        }

        // check if the operation type is valid
        if (op_type != 'l' && op_type != 's') {
            std::cerr << "Error: Invalid operation type '" << op_type << "'. Expected 'l' or 's'." << std::endl;
            exit(1);
        }

        // create the operation object
        Operation op;
        op.is_load = (op_type == 'l');
        op.address = address;
        operations.push_back(op);
    }

    return operations;
}

// Getter to extract Address = {tag, index, and offset}, we can assume address is 32 bit
Address decompose_address(uint32_t address, Config config) { 
    // Offset needs to be able to represent block_size, so if 
    // block_size is 16 = 4 bits = can represent 0 to 15. 
    Address addr;
    addr.offset = address & (config.block_size - 1); // 16 - 1 = 15 = 1111
    addr.index = (address / config.block_size) & (config.num_sets - 1); 
    addr.tag = address / (config.block_size * config.num_sets); // Get the remaining bits for tag

    return addr;
}

// Load: Cache structure: Cache -> Set -> Slot
void load_address(Address addr, Cache& cache, Config config, Stats& stats, uint32_t global_time) {
    // Hit (valid && tag == addr.tag in one of the slots in the set)
    Set& set = cache.sets[addr.index];
    for (auto& slot : set.slots) {
        if (slot.valid && slot.tag == addr.tag) {
            stats.load_hits++;
            slot.access_ts = global_time;
            stats.total_cycles += 1; // Hit takes 1 cycle
            return;
        }
    }

    // Miss
    stats.total_cycles += 1 + 100 * (config.block_size/4); // load takes 1 and miss search takes 100
    stats.load_misses++;
    // Try to find an empty slot first
    for (auto& slot : set.slots) {
        if (!slot.valid) {
            slot.valid = true;
            slot.tag = addr.tag;
            slot.dirty = false;
            slot.load_ts = global_time;
            slot.access_ts = global_time;
            return;
        }
    }
    // No empty slot, find which to evict (LRU or FIFO)
    Slot *evict_slot = &set.slots[0]; // this is pointer 
    for (auto& slot : set.slots) {
        if (config.lru) {
            // Evict the least recently used slot
            if (slot.access_ts < evict_slot->access_ts) {
                evict_slot = &slot;
            }
        } else {
            // Evict the first-in slot
            if (slot.load_ts < evict_slot->load_ts) {
                evict_slot = &slot;
            }
        }
    }
    // If the evicted slot is dirty, we need to write it back to memory, so more cycles needed
    if (evict_slot->dirty) {
        stats.total_cycles += 100 * (config.block_size/4);
    } 

    // Evict the slot
    evict_slot->valid = true;
    evict_slot->tag = addr.tag;
    evict_slot->dirty = false;
    evict_slot->load_ts = global_time;
    evict_slot->access_ts = global_time;
}

// Store: Cache structure: Cache -> Set -> Slot
void store_address(Address addr, Cache& cache, Config config, Stats& stats, uint32_t global_time) {
    // Hit
    Set& set = cache.sets[addr.index];
    for (auto& slot : set.slots) {
        if (slot.valid && slot.tag == addr.tag) {
            stats.store_hits++;
            slot.access_ts = global_time;
            stats.total_cycles += 1; // Hit takes 1 cycle
            // If write-through, we write 4 bytes directly, so 100 cycles
            if (config.write_through) { 
                stats.total_cycles += 100;
            } else {
                slot.dirty = true; // Mark dirty for write-back
            }
            return;
        }
    }
    // Miss
    stats.total_cycles += 1; // Miss search takes 1 cycle
    stats.store_misses++;

    // if write allocate
    if (config.write_allocate) { 
        // Try to find an empty slot first
        for (auto& slot : set.slots) {
            if (!slot.valid) {
                slot.valid = true;
                slot.tag = addr.tag;
                slot.dirty = !config.write_through; // Mark dirty if write-back
                slot.load_ts = global_time;
                slot.access_ts = global_time;
                stats.total_cycles += 100 * (config.block_size/4); // load takes 100 cycles
                return;
            }
        }
        // No empty slot, find which to evict (LRU or FIFO)
        Slot *evict_slot = &set.slots[0]; // this is pointer 
        for (auto& slot : set.slots) {
            if (config.lru) {
                // Evict the least recently used slot
                if (slot.access_ts < evict_slot->access_ts) {
                    evict_slot = &slot;
                }
            } else {
                // Evict the first-in slot
                if (slot.load_ts < evict_slot->load_ts) {
                    evict_slot = &slot;
                }
            }
        }
        // If the evicted slot is dirty, we need to write it back to memory, so more cycles needed
        if (evict_slot->dirty) {
            stats.total_cycles += 100 * (config.block_size/4);
        } 

        // Evict the slot and load the new block
        evict_slot->valid = true;
        evict_slot->tag = addr.tag;
        evict_slot->dirty = !config.write_through; // Mark dirty if write-back
        evict_slot->load_ts = global_time;
        evict_slot->access_ts = global_time;
        stats.total_cycles += 100 * (config.block_size/4); // load takes 100 cycles

    // If no write allocate: just take time to write to memory
    } else {
        stats.total_cycles += 100;
    }
}
  



