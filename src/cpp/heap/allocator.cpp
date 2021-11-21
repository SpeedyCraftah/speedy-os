#include "allocator.h"
#include "../scheduling/scheduler.h"
#include <stddef.h>

// Overload heap allocation operators.
void* operator new(size_t size) {
    return heap::malloc(size);
}

void* operator new[](size_t size) {
    return heap::malloc(size);
}

void operator delete(void* ptr, size_t size) {
    heap::free(ptr);
};

uint32_t heap::get_allocated_bytes() {
    return total_reserved_bytes;
}

uint32_t heap::get_total_blocks() {
    return total_blocks;
}

uint32_t heap::get_reserved_blocks() {
    return reserved_blocks;
}

void* heap::malloc(uint32_t size, bool reset, bool skip_reuse, uint32_t process_id) {
    // For temporary use.
    if (process_id == 0) process_id = scheduler::current_process;
    
    // If blocks exist and allocation allows block reuse.
    if (total_blocks != 0 && !skip_reuse) {
        for (uint32_t i = 0; i <= total_blocks; i++) {
            record meta = data_meta[i];
            
            // If block is already reserved.
            if (meta.reserved) continue;

            // Safety check.
            if (meta.location == 0) continue;

            // If size is too small or too large.
            if (size > meta.size || meta.size - size > 30) continue;

            // Override block.
            data_meta[i].process_id = process_id;
            data_meta[i].reserved = true;

            // If contents should be reset.
            if (reset) {
                for (uint32_t i = 0; i < meta.size; i++) {
                    meta.location[i] = 0;
                }
            }

            // Update statistics.
            total_reserved_bytes += meta.size;
            reserved_blocks += 1;

            return meta.location;
        }
    }

    // If no satisfactory existing block was found.

    // Create new block.
    uint32_t block_id = ++total_blocks;

    data_meta[block_id].location = next_data_location;
    data_meta[block_id].size = size;
    data_meta[block_id].process_id = process_id;
    data_meta[block_id].reserved = true;

    // Update statistics.
    total_reserved_bytes += size;
    reserved_blocks += 1;

    // Set next location.
    next_data_location += size + 1;

    return data_meta[block_id].location;
}

bool heap::free(void* ptr) {
    for (uint32_t i = 0; i <= total_blocks; i++) {
        record meta = data_meta[i];

        // If block isn't reserved.
        if (!meta.reserved) continue;
        
        // If address does not match block.
        if (meta.location != ptr) continue;

        // Free block.
        data_meta[i].reserved = false;
        data_meta[i].process_id = 0;

        // Update statistics.
        total_reserved_bytes -= meta.size;
        reserved_blocks -= 1;

        return true;
    }

    return false;
}

void heap::free_by_process_id(uint32_t process_id) {
    for (uint32_t i = 0; i <= total_blocks; i++) {
        record meta = data_meta[i];

        // If block isn't reserved.
        if (!meta.reserved) continue;
        
        // If address is not owned by process.
        if (meta.process_id != process_id) continue;

        // Free block.
        data_meta[i].reserved = false;
        data_meta[i].process_id = 0;

        // Update statistics.
        total_reserved_bytes -= meta.size;
        reserved_blocks -= 1;
    }
}