#pragma once

#include "paging.h"
#include "stdint.h"
#include "../scheduling/structures/process.h"

namespace virtual_allocator {
    bool read_virtual_memory(Process* process, void* virtual_address_start, uint32_t size, uint8_t* out, bool allow_kernel_pages = false);
    PageEntry* fetch_page_index(Process* process, uint32_t index, bool allocate_directory = true);
    uint32_t find_free_virtual_pages(Process* process, uint32_t count);
    bool preferred_virtual_pages_free(Process* process, uint32_t virtual_index_start, uint32_t count);
    void reset_virtual_pages(Process* process, uint32_t virtual_index_start, uint32_t count, bool deallocate_page);
}