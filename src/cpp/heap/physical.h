#pragma once

#include "stdint.h"
#include "../scheduling/structures/process.h"
#include "../paging/paging.h"

namespace physical_allocator {
    // Define location of meta data array.
    static bool* data_meta = (bool*)0x01000000;

    // Statistics.
    static uint32_t reserved_blocks = 0;

    // Location of heap data.
    static uint8_t* data_location = (uint8_t*)0x1200000;
    static uint32_t page_index_start = 0x1200000 / 4096;

    void init();

    // Functions.
    void* alloc_physical_page(uint32_t count = 1, bool reset = false);
    bool free_physical_page(void* address);
 
    uint32_t get_allocated_bytes();
};