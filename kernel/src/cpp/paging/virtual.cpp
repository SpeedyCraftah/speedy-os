#include "virtual.h"
#include "paging.h"
#include "../heap/physical.h"
#include "../misc/memory.h"

extern "C" volatile int bpwatch;

namespace virtual_allocator {
    // TODO - check pages for validity beforehand.
    bool read_virtual_memory(Process* process, void* virtual_address_start, uint32_t size, uint8_t* out, bool allow_kernel_pages) {
        uint32_t page_index = paging::address_to_pi(virtual_address_start);
        uint32_t remaining_bytes_in_page = 4096 - (reinterpret_cast<uint32_t>(virtual_address_start) % 4096);
        uint32_t remaining_bytes = size;
        uint32_t bytes_to_read = size > remaining_bytes_in_page ? remaining_bytes_in_page : size;
        
        PageEntry* page;
        uint8_t* physical;

        page = fetch_page_index(process, page_index, false);
        if (page == nullptr || (!allow_kernel_pages && !page->UserSupervisor)) return false;
        physical = (uint8_t*)paging::pi_to_address(page->Address) + (reinterpret_cast<uint32_t>(virtual_address_start) % 4096);

        // Read the first page until the end.
        memcpy(out, physical, bytes_to_read);
        out += bytes_to_read;
        remaining_bytes -= bytes_to_read;
        page_index++;

        // Read pages in the middle.
        while (remaining_bytes >= 4096) {
            page = fetch_page_index(process, page_index, false);
            if (page == nullptr || (!allow_kernel_pages && ((page->KernelFlags & PageFlags::PROCESS_OWNED) == 0))) return false;
            physical = (uint8_t*)paging::pi_to_address(page->Address);

            // Read the whole page.
            memcpy(out, physical, 4096);
            out += 4096;
            remaining_bytes -= 4096;
            page_index++;
        }

        // Read the last remaining bytes if needed.
        if (remaining_bytes != 0) {
            page = fetch_page_index(process, page_index, false);
            if (page == nullptr || (!allow_kernel_pages && ((page->KernelFlags & PageFlags::PROCESS_OWNED) == 0))) return false;
            physical = (uint8_t*)paging::pi_to_address(page->Address) + (reinterpret_cast<uint32_t>(virtual_address_start) % 4096);

            memcpy(out, physical, remaining_bytes);
        }

        return true;
    }

    PageEntry* fetch_page_index(Process* process, uint32_t index, bool allocate_directory) {
        PageDirectory& directory = process->paging.directories[index / 1024];
        
        if (!directory.Present) {
            if (allocate_directory) {
                PageEntry* table = (PageEntry*)physical_allocator::alloc_physical_page(1, true);
                directory.Address = paging::address_to_pi(table);
                directory.Present = true;
                directory.UserSupervisor = true;

                return &table[index % 1024];
            } else return nullptr;
        }

        PageEntry* table = (PageEntry*)paging::pi_to_address(directory.Address);
        return &table[index % 1024];
    }

    uint32_t find_free_virtual_pages(Process* process, uint32_t count) {
        // TODO - add boundary checks
        // Start at 100MB.
        uint32_t offset = 104857600 / 4096;
    
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
        return offset;
    }

    bool preferred_virtual_pages_free(Process* process, uint32_t virtual_index_start, uint32_t count) {
        // TODO - add boundary checks
        uint32_t offset = virtual_index_start;
    
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, offset + i, false);

            // Check if entry is available.
            if (entry != nullptr && *(uint32_t*)entry != 0) return false;
        }

        // Consecutive entries are free.
        return true;
    }

    void reset_virtual_pages(Process* process, uint32_t virtual_index_start, uint32_t count, bool deallocate_page) {
        for (uint32_t i = 0; i < count; i++) {
            PageEntry* entry = fetch_page_index(process, virtual_index_start, false);
            if (entry == nullptr) continue;

            if (deallocate_page) physical_allocator::free_physical_page(paging::pi_to_address(entry->Address));
            *(uint32_t*)entry = 0;
        }
    }

    
}
