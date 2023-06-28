#include "physical.h"
#include "../scheduling/structures/process.h"
#include "../io/video.h"

extern "C" volatile int bpwatch = 0;

namespace physical_allocator {
    void init() {
        // Set the data meta entries to zero.
        for (uint32_t i = 0; i < (reinterpret_cast<uint32_t>(data_location) - reinterpret_cast<uint32_t>(data_meta)) / 4; i++) {
            *(uint32_t*)(data_meta + i) = 0;
        }
    }

    void* alloc_physical_page(uint32_t count, bool reset) {
        uint32_t offset = 0;

        // Find free page(s).
        find_blocks_loop:
        for (uint32_t i = 0; i < count; i++) {
            // Check if entry is available.
            if (!data_meta[offset + i]) continue;
            else {
                offset += i + 1;
                goto find_blocks_loop;
            }
        }

        // Allocate the page(s).
        for (uint32_t i = 0; i < count; i++) {
            data_meta[offset + i] = true;
        }

        uint32_t* data = (uint32_t*)(data_location + (offset * 4096));

        // Wipe the area clear if needed.
        if (reset) {
            for (uint32_t i = 0; i < 1024 * count; i++) {
                data[i] = 0;
            }
        }

        reserved_blocks++;

        return data;
    }

    bool free_physical_page(void* address) {
        uint32_t index = (reinterpret_cast<uint32_t>(address) - reinterpret_cast<uint32_t>(data_location)) / 4096;
        bool old_val = data_meta[index];
        data_meta[index] = false;
        
        return old_val;
    }
};