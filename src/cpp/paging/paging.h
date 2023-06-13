#pragma once

#include "stdint.h"
//#include "../tables/pagedir.h"

struct PageDirectory {
    uint32_t Present : 1;
    uint32_t ReadWrite : 1;
    uint32_t UserSupervisor : 1;
    uint32_t CacheWriteThrough : 1;
    uint32_t CacheDisable : 1;
    uint32_t Accessed : 1;
    uint32_t NotUsed0 : 1;
    uint32_t PageSize : 1;
    uint32_t NotUsed1 : 4;
    uint32_t Address : 20;
} __attribute__((packed));

enum KernelPageFlags : uint32_t {
    PROCESS_RESERVED = 1, // This page was reserved or is owned by the process and should be released when the process ends. Otherwise it is owned by the kernel and should not be released.
};

enum PageAllocationFlags : uint32_t {
    ALLOCATE_IMMEDIATELY = 1, // Virtual address space is reserved but physical page will only be allocated on access (page fault).
};

enum PageFreeFlags : uint32_t {
    ONLY_FREE_PHYSICAL = 1 // Virtual address will not be freed but if a physical page is allocated it will be freed.
};

struct PageEntry {
    uint32_t Present : 1;
    uint32_t ReadWrite : 1;
    uint32_t UserSupervisor : 1;
    uint32_t CacheWriteThrough : 1;
    uint32_t CacheDisable : 1;
    uint32_t Accessed : 1;
    uint32_t Dirty : 1;
    uint32_t PageAttributeTable : 1;
    uint32_t Global : 1;
    uint32_t KernelFlags : 3;
    uint32_t Address : 20;
} __attribute__((packed));

namespace paging {
    inline void enable_pcid() {
        asm volatile(
            "mov %%cr4, %%eax\n"
            "or $0x00000010, %%eax\n"
            "mov %%eax, %%cr4\n"
            :
            :
            : "eax"
        );
    }

    inline void set_process_context_id(uint32_t pcid) {
        asm volatile(
            "mov %0, %%eax\n"
            "mov %%eax, %%cr3\n"
            :
            : "r" (pcid)
            : "eax"
        );
    }

    inline void enable(PageDirectory* address) {
        asm volatile(
            "mov %0, %%cr3\n"
            "mov %%cr0, %%eax\n"
            "or $0x80000000, %%eax\n"
            "mov %%eax, %%cr0\n"
            :
            : "r" (address)
            : "eax"
        );
    }

    inline void disable() {
        asm volatile(
            "mov %%cr0, %%eax\n"
            "and $0x7FFFFFFF, %%eax\n"
            "mov %%eax, %%cr0\n"
            :
            :
            : "eax"
        );
    }

    // Conversion functions.
    inline uint32_t address_to_pi(void* address) {
        return reinterpret_cast<uint32_t>(address) >> 12;
    }

    inline void* pi_to_address(uint32_t pi) {
        return reinterpret_cast<void*>(pi << 12);
    }
}