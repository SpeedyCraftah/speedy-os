#include "elf.h"
#include "structures/elf.h"
#include "../scheduling/scheduler.h"
#include "../paging/virtual.h"
#include "../heap/physical.h"
#include "../misc/memory.h"

extern "C" volatile int bpwatch;

namespace loader {
    Process* load_elf32_executable_as_process(char* name, ProcessFlags flags, void* elf_start, void* elf_end) {
        // TODO - check header end to prevent overrun
        Elf32Header* header = (Elf32Header*)elf_start;
        
        // Check signature and architecture.
        if (header->magic_number != ELF32_MAGIC_NUMER || header->bits != Elf32Header::Bits::X32 || header->architecture != Elf32Header::Arch::X86) {
            return nullptr;
        }

        // Check for ELF type, only executable is supported at the moment.
        if (header->elf_type != Elf32Header::Type::EXECUTABLE) {
            return nullptr;
        }

        // Check if entry point is null.
        if (header->virtual_entry_address == 0) return nullptr;

        uint8_t* program_header_start = reinterpret_cast<uint8_t*>(elf_start) + header->program_header_table_offset;

        // Check if there is at least one loadable segment.
        bool loadable_segment_present = false;
        for (uint32_t i = 0; i < header->program_header_entry_count; i++) {
            Elf32ProgramHeader* program_header = (Elf32ProgramHeader*)(program_header_start + (header->program_header_entry_size * i));
            if (program_header->segment_type == Elf32ProgramHeader::SegmentType::LOADABLE) {
                if (program_header->data_required_alignment != 4096) return nullptr;
                loadable_segment_present = true;
                break;
            }
        }

        // If there is no loadable segment.
        if (!loadable_segment_present) return nullptr;

        // Create the process instance.
        Process* process = scheduler::create_process(name, (void(*)())header->virtual_entry_address, flags, true);

        // Iterate and load the loadable segments.
        for (uint32_t i = 0; i < header->program_header_entry_count; i++) {
            Elf32ProgramHeader* program_header = (Elf32ProgramHeader*)(program_header_start + (header->program_header_entry_size * i));
            if (program_header->segment_type != Elf32ProgramHeader::SegmentType::LOADABLE) continue;

            // Calculate physical/virtual pages required for this segment.
            uint32_t virtual_pi_start = paging::address_to_pi(program_header->virtual_start_address);
            uint32_t pages_required = (program_header->segment_memory_size / 4096) + (program_header->segment_memory_size % 4096 != 0);

            // Check if the pages can be loaded in the specified position.
            if (!virtual_allocator::preferred_virtual_pages_free(process, virtual_pi_start, pages_required)) {
                scheduler::kill_process(process, 1);
                return nullptr;
            }

            // TODO - physical pages do not have to be consecutive.
            void* physical_pages = physical_allocator::alloc_physical_page(pages_required, true);
            if (physical_pages == nullptr) {
                kernel::panic("Could not allocate required physical pages for elf section.");
                __builtin_unreachable();
            }

            uint32_t physical_pi_start = paging::address_to_pi(physical_pages);

            // Copy the data of elf section to the physical page.
            uint8_t* data = reinterpret_cast<uint8_t*>(elf_start) + program_header->data_offset;
            memcpy(physical_pages, data, program_header->segment_size);

            // If the rest of the section size in memory needs to be zeroed.
            //uint32_t zero_count = program_header->segment_memory_size - program_header->segment_size;
            //if (zero_count != 0) memset((uint8_t*)physical_pages + program_header->segment_size, 0, zero_count);

            // Map the physical pages to virtual address space at specified locations.
            for (uint32_t j = 0; j < pages_required; j++) {
                PageEntry* entry = virtual_allocator::fetch_page_index(process, virtual_pi_start + j);
                entry->Present = true;
                entry->Address = physical_pi_start + j;
                entry->KernelFlags = PageFlags::RELEASE_PHYSICAL_ON_EXIT;
                entry->UserSupervisor = true;
                entry->ReadWrite = program_header->flags.WRITABLE;
            }
        }

        // Schedule the process for execution.
        //scheduler::thread_execution_queue->push(process->threads->peek_front());

        // Return the process.
        return process;
    }
}