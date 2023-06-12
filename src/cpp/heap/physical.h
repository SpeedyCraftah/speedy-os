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

    // Process functions.
    PageEntry* fetch_page_index(Process* process, uint32_t index, bool allocate_directory = false);
    uint32_t alloc_virtual_pages(Process* process, uint32_t count = 1, uint32_t flags = 0, bool kernel_allocated = false);
    void map_virtual_pages(Process* process, uint32_t physical_index_start, uint32_t virtual_index_start, uint32_t count);
    uint32_t alloc_virtual_mmped_pages(Process* process, uint32_t physical_pi_start, uint32_t count, uint32_t flags = 0);

    bool free_virtual_page(Process* process, uint32_t pi, uint32_t count = 0, uint32_t flags = 0);
 
    uint32_t get_allocated_bytes();
};