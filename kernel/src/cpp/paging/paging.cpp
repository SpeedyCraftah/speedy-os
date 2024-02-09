#include "paging.h"
#include "../io/video.h"
#include "../../../../shared/conversions.h"
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

void log_fault_details(PageFaultAddress address, PageFaultError error) {
    char hex[HEX32_STR_SIZE];

    constexpr rgb_colour colour = rgb_colour(0, 0xAF, 0xFF);

    video::printf("\nAdditional fault details:\n", colour);
    video::printf("  operation=", colour);
    video::printf((char*)(error.Write ? "WRITE\n" : "READ\n"), colour);
    video::printf("  instruction=", colour);
    video::printf(conversions::u_int32_to_hex(scheduler::current_thread->registers->eip, hex), colour);
    video::printf("\n  page address=", colour);
    video::printf(conversions::u_int32_to_hex((address.DirectoryEntry * 1024) * 4096 + (address.TableEntry * 4096), hex), colour);
    video::printf("\n  flags=", colour);
    
    if (error.InstructionFetch) video::printf("InstructionFetch ", colour);
    if (error.Present) video::printf("Present ", colour);
    if (error.User) video::printf("IOPL3 ", colour);
    video::printnl();
}

uint32_t HandlePageFault_hl(PageFaultAddress address, PageFaultError error) {
    // panic glitches out - fix
    //kernel::panic("Page fault");

    /*if (!error.User) [[unlikely]] {
        kernel::panic("A page fault has occurred at ring 0.");
        __builtin_unreachable();
    }*/

    PageDirectory* directories = scheduler::current_thread->process->paging.directories;
    PageDirectory& directory = directories[address.DirectoryEntry];

    if (!directory.Present) {
        scheduler::kill_process(scheduler::current_thread->process, ProcessTermCode::PAGE_ACCESS_VIOLATION);
        video::printf("Process accessed a non existent directory!", VGA_COLOUR::LIGHT_RED);
        log_fault_details(address, error);
        return 1;
    }

    PageEntry* table = (PageEntry*)paging::pi_to_address(directory.Address);
    PageEntry& entry = table[address.TableEntry];

    // If page exists and needs to be allocated physical space.
    if (!entry.Present) {
        if ((entry.KernelFlags & PageFlags::DYNAMIC_PAGE_ALLOCATION) != 0) {
            void* physical_page = physical_allocator::alloc_physical_page(1, true);
            if (physical_page == nullptr) {
                scheduler::kill_process(scheduler::current_thread->process, ProcessTermCode::PAGE_ACCESS_VIOLATION);
                video::printf("Could not allocate dynamic physical page for process!", VGA_COLOUR::LIGHT_RED);
                return 1;
            }

            entry.Present = true;
            entry.Address = paging::address_to_pi(physical_page);

            return 0;
        } else {
            scheduler::kill_process(scheduler::current_thread->process, ProcessTermCode::PAGE_ACCESS_VIOLATION);
            video::printf("Process accessed a non existent page!", VGA_COLOUR::LIGHT_RED);
            log_fault_details(address, error);
            return 1;
        }
    }

    // Other page fault reasons - read only, privilege level, etc.
    scheduler::kill_process(scheduler::current_thread->process, ProcessTermCode::PAGE_ACCESS_VIOLATION);
    video::printf("Process page access triggered a page fault!", VGA_COLOUR::LIGHT_RED);
    log_fault_details(address, error);
    return 1;
}

// TODO: SORT OUT CACHE INVALIDATION FOR PAGING
// to get virtual address: ((address.DirectoryEntry * 1024) + address.TableEntry) * 4096)
extern "C" uint32_t HandlePageFault(PageFaultAddress address, PageFaultError error) {
    // Switch to kernel page directory.
    paging::switch_directory(paging::kernel_page_directory);
    
    uint32_t result = HandlePageFault_hl(address, error);
    if (result == 0) {
        // Switch back to process page directory.
        paging::switch_directory(scheduler::current_thread->process->paging.directories);
    }

    return result;
}