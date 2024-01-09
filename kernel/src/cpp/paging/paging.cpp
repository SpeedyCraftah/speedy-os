#include "paging.h"
#include "../io/video.h"
#include "../misc/conversions.h"
#include "../panic/panic.h"
#include "../scheduling/scheduler.h"
#include "../heap/physical.h"
#include "virtual.h"

extern "C" __attribute__((fastcall)) void enable_paging_asm(PageDirectory* address);
__attribute__((aligned(4096))) PageDirectory paging::kernel_page_directory[1024] = {};


extern "C" volatile int bpwatch;

struct PageFaultError {
    uint32_t Present : 1;
    uint32_t Write : 1;
    uint32_t User : 1;
    uint32_t RSVD : 1;
    uint32_t InstructionFetch : 1;
    uint32_t PK : 1;
    uint32_t SS : 1;
    uint32_t Reserved0 : 8;
    uint32_t SGX : 1;
    uint32_t Reserved1 : 16;
} __attribute__((packed));

struct PageFaultAddress {
    uint32_t Unused0 : 12;
    uint32_t TableEntry : 10;
    uint32_t DirectoryEntry : 10;
} __attribute__((packed));

uint32_t HandlePageFault_hl(PageFaultAddress address, PageFaultError error) {
    // panic glitches out - fix
    //kernel::panic("Page fault");

    video::printf("Page fault!", VGA_COLOUR::LIGHT_RED);
    asm volatile("cli; hlt");

    if (!error.User) [[unlikely]] {
        kernel::panic("A page fault has occurred at ring 0.");
        __builtin_unreachable();
    }

    // If page is present.
    if (error.Present) {
        // kill process
    } else {
        PageDirectory* directories = scheduler::current_thread->process->paging.directories;
        PageDirectory& directory = directories[address.DirectoryEntry];

        if (!directory.Present) {
            // kill process
        }

        PageEntry* table = (PageEntry*)paging::pi_to_address(directory.Address);
        PageEntry& entry = table[address.TableEntry];

        // If page exists and needs to be allocated physical space.
        
    }
}

// TODO: SORT OUT CACHE INVALIDATION FOR PAGING
// to get virtual address: ((address.DirectoryEntry * 1024) + address.TableEntry) * 4096)
extern "C" uint32_t HandlePageFault(PageFaultAddress address, PageFaultError error) {
    // Switch to kernel page directory.
    if (!scheduler::current_thread->process->flags.system_process) paging::switch_directory(paging::kernel_page_directory);
    
    uint32_t result = HandlePageFault_hl(address, error);
    if (result == 0) {
        // Switch back to process page directory.
        if (!scheduler::current_thread->process->flags.system_process) paging::switch_directory(scheduler::current_thread->process->paging.directories);
    }

    return result;
}