#include "physical.h"
#include "../scheduling/structures/process.h"
#include "../io/video.h"

extern "C" void debug();

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

    // TODO - Optimise.
    PageEntry* fetch_page_index(Process* process, uint32_t index, bool allocate_directory) {
        PageDirectory& directory = process->paging.directories[index / 1024];
        
        if (!directory.Present) {
            if (allocate_directory) {
                PageEntry* table = (PageEntry*)physical_allocator::alloc_physical_page(1, true);
                directory.Address = paging::address_to_pi(table);
                directory.Present = true;

                return &table[index % 1024];
            } else return nullptr;
        }

        bpwatch = 6;
        PageEntry* table = (PageEntry*)paging::pi_to_address(directory.Address);
        return &table[index % 1024];
    }

    uint32_t alloc_virtual_pages(Process* process, uint32_t count, uint32_t flags, bool kernel_allocated) {
        // TODO - add boundary checks
        // Start at 100MB.
        uint32_t offset = 104857600 / 4096;

        /*
        while (true) {
            bool no_space = false;
            for (uint32_t i = 0; i < count; i++) {
                PageEntry* entry = fetch_page_index(process, offset + i, true);
                if (entry == nullptr) [[unlikely]] {
                    kernel::panic("Fetch page index lookup failed!");
                }

                // Check if entry is available.
                if (*(uint32_t*)entry == 0) continue;
                else {
                    offset += i + 1;
                    no_space = true;
                    break;
                }
            }

            if (!no_space) break;
        }
        */
    
        find_entry_loop:
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, offset + i, true);

            // Check if entry is available.
            if (*(uint32_t*)entry == 0) continue;
            else {
                offset += i + 1;
                goto find_entry_loop;
            }
        }

        // Consecutive entries were found.
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, offset + i, true);
            entry->UserSupervisor = true;
            entry->KernelFlags = kernel_allocated ? KernelPageFlags::PROCESS_RESERVED : 0;
            entry->Present = false;
        }

        // Return start of virtual page addresses index.
        return offset;
    }

    // TODO - remove some checks in favour of debug only checks
    void map_virtual_pages(Process* process, uint32_t physical_index_start, uint32_t virtual_index_start, uint32_t count) {
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, virtual_index_start + i, false);
            if (entry == nullptr) [[unlikely]] {
                kernel::panic("Tried to map virtual page to physical page, but virtual page was not allocated.");
            }

            entry->Present = true;
            entry->Address = physical_index_start + i;
        }
    }

    uint32_t alloc_virtual_mmped_pages(Process* process, uint32_t physical_pi_start, uint32_t count, uint32_t flags) {
        uint32_t pages = alloc_virtual_pages(process, count, 0, true);
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* page = fetch_page_index(process, pages + i);
            page->Address = physical_pi_start++;
            page->Present = true;
        }

        return pages;
    }

    bool free_virtual_page(Process* process, uint32_t pi, uint32_t count, uint32_t flags) {
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, pi);
            if (entry == nullptr) continue;

            // Free the physical page (if allocated).
            bool result = 
                entry->Present ? free_physical_page(paging::pi_to_address(entry->Address)) : true;

            // Set the page entry to empty.
            if ((flags & PageFreeFlags::ONLY_FREE_PHYSICAL) == 0) *(uint32_t*)entry = 0;
            else {
                // Otherwise only release the physical page.
                entry->Present = false;
            }
        }

        return true;
    }
};