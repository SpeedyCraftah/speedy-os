#pragma once

#include "stdint.h"

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

enum PageFlags : uint32_t {
    PROCESS_OWNED = 1, // This page was reserved or is owned by the process and should be released when the process ends. Otherwise it is owned by the kernel and should not be released.
    DYNAMIC_PAGE_ALLOCATION = 2, // Upon access by the process assuming proper read/write privileges, a physical page will be allocated, mapped and will return from the page fault.
    RELEASE_PHYSICAL_ON_EXIT = 4 // Explicit version of PROCESS_OWNED, will release the page regardless if it is owned by the kernel or not.
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
    extern __attribute__((aligned(4096))) PageDirectory kernel_page_directory[1024];

    inline void switch_directory(PageDirectory* address) {
        asm volatile(
            "mov %0, %%cr3"
            :
            : "r" (address)
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