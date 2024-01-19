#include "syscalls.h"

#include "datasink.h"
#include "events.h"
#include "scheduler.h"

#include "../software/system/speedyshell/main.h"
#include "structures/datasink.h"
#include "structures/events.h"
#include "structures/process.h"
#include "structures/thread.h"
#include "../chips/pit.h"
#include "../heap/physical.h"
#include "../misc/memory.h"
#include "../paging/virtual.h"

#include "../kernel.h"

//temp
#include "../misc/conversions.h"

extern "C" Registers* temporary_registers;

// ID 0 = Do nothing (nop).
// ID 1 = Query kernel for data.
//      - 0 = Elapsed time since startup.
//      - 1 = Process ID of process.
//      - 2 = Thread ID of thread.
// ID 2 = End process request (with status code). 
// ID 3 = Temporarily suspend a thread with specific conditions.
// ID 6 = End of event call.
// ID 7 = Register event for process.
// ID 8 = Emit event for registered processes.
// ID 9 = Change program status.
// ID 10 = Find process ID of string.
// ID 14 = Get hardware-supported random 32-bit number.
// ID 15 = Create a new thread.
// ID 16 = Kill a thread.
// ID 17 = Park a thread.
// ID 18 = Unpark a thread.
// ID 19 = Upgrade the graphics mode.
// ID 20 = Voluntarily preempt the execution.
// ID 21 = Allocate x virtual pages with flags.
// ID 22 = Free a virtual page with flags.
// ID 23 = Write to data sink.

// SpeedyShell Only
// ID 11 = Query SpeedyShell for data.
//       - 0 = Get whole command string.
// ID 12 = Print text to terminal with colour.
// ID 13 = Get input from user.

// Return 0 = Resume program execution.
// Return 1 = Halt and wait for next interrupt.

// Param Order: ECX (ID), EDX, EAX, EBX, Stack...

// to add
/*
new param for data (1)

*/

extern "C" volatile int bpwatch;

enum SyscallPageAllocationFlag : uint32_t {
    ALLOCATE_ON_ACCESS = 1, // Virtual address space is reserved but physical page will only be allocated on access (page fault).
    ALLOCATE_IMMEDIATELY = 2, // Physical page will be allocated immediately on request alongside the virtual page.
};

uint32_t handle_system_call_hl() {
    uint32_t id = temporary_registers->ecx;
    uint32_t data = temporary_registers->edx;

    if (id == 0) {
        // my birthday :)
        temporary_registers->eax = 3009;
    } else if (id == 1) {
        if (data == 0) temporary_registers->eax = scheduler::elapsed_ms;
        else if (data == 1) temporary_registers->eax = scheduler::current_thread->process->id;
        else if (data == 2) temporary_registers->eax = scheduler::current_thread->id;
    } else if (id == 2) {
        scheduler::kill_process(scheduler::current_thread->process, data);
        return 1;
    } else if (id == 3) {
        // May add conditional suspensions.
        // May add process wide suspensions.

        // Add CPU time.
        uint32_t used_cpu_time = scheduler::time_slice_ms - (uint32_t)chips::pit::fetch_channel_0_remaining_countdown();
        scheduler::current_thread->process->total_cpu_time += used_cpu_time;
        scheduler::scheduler_running_time_ms += used_cpu_time;

        // Apply suspension.
        scheduler::current_thread->state.suspended = true;
        scheduler::current_thread->suspension_details.resume_time =
            data == 0 ? 1 : scheduler::elapsed_ms + data;

        // Save state and reschedule.
        scheduler::manual_context_switch_return();

        return 1;
    } else if (id == 6) {
        // If there is no event currently running, return.
        if (!scheduler::current_thread->state.processing_event) return 0;

        uint32_t cpu_time_used = scheduler::time_slice_ms - (uint32_t)chips::pit::fetch_channel_0_remaining_countdown();

        // Add CPU time.
        scheduler::current_thread->process->total_cpu_time += cpu_time_used;
        scheduler::scheduler_running_time_ms += cpu_time_used;

        // Clear event flag.
        scheduler::current_thread->state.processing_event = false;

        // Restore registers.
        memcpy(scheduler::current_thread->registers, &scheduler::current_thread->backup_registers, sizeof(Registers));

        // Schedule the thread.
        scheduler::thread_execution_queue->push(scheduler::current_thread);

        // Clear current thread.
        scheduler::current_thread = nullptr;
        scheduler::temporary_registers = &scheduler::placeholder_registers;

        return 1;
    } else if (id == 7) {
        // May be moved to a function in events.cpp.
        uint32_t data2 = temporary_registers->eax;
        uint32_t data3 = temporary_registers->ebx;

        // Check if process exists.
        if (!scheduler::process_list->exists(data)) {
            temporary_registers->eax = 0;
            return 0;
        }

        Process* emitter_process = scheduler::process_list->fetch(data);

        // Create event listener.
        ThreadEventListener listener;
        listener.enabled_event_ids = data2;
        listener.thread_id = scheduler::current_thread->id;
        listener.handler = reinterpret_cast<void*>(data3);
        
        // Register the event listener.
        emitter_process->hooked_threads->push(listener);

        // Return success response.
        temporary_registers->eax = 1;
    } else if (id == 8) {
        uint32_t data2 = temporary_registers->eax;

        scheduler::events::emit_event(scheduler::current_thread->process, data, data2);
    } else if (id == 9) {
        // Future: end event if processing event.
        uint32_t data2 = temporary_registers->eax;

        // Check if execution policy is valid.
        if (data > 2) return 0;

        // If a different thread is referenced.
        if (data2 != 0 && scheduler::current_thread->id != data2) {
            if (!scheduler::thread_list->exists(data2)) return 0;

            // Fetch the thread.
            Thread* thread = scheduler::thread_list->fetch(data2);

            // Change the execution policy.
            thread->execution_policy = (ThreadExecutionPolicy)data;

            return 0;
        }

        // Change the execution policy.
        scheduler::current_thread->execution_policy = (ThreadExecutionPolicy)data;

        // Keep long syntax for future.
        // If thread is running normally and is set to event only, yield.
        if (data == ThreadExecutionPolicy::EVENT_ONLY && !scheduler::current_thread->state.processing_event) {
            // Save thread data and yield.
            scheduler::manual_context_switch_return();

            return 1;
        }
    } else if (id == 10) {
        // If process name string does not exist, return.
        if (!scheduler::process_name_list->exists((char*)data)) {
            temporary_registers->eax = 0;

            return 0;
        }

        Process* process = scheduler::process_name_list->fetch((char*)data);
        temporary_registers->eax = process->id;
    } else if (id == 11) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            temporary_registers->eax = 0;
            return 0;
        }

        // To-do: copy text instead of direct pointer.
        char* input = speedyshell::text_buffer;
        temporary_registers->eax = reinterpret_cast<uint32_t>(input);
    } else if (id == 12) {
        // If interface method is not SpeedyShell or graphics mode is pixel.
        /*if (!isTerminalInterface || speedyshell::pixel_mode) {
            temporary_registers->eax = 0;
            return 0;
        }*/
        // TODO - temporary, reverse once speedyshell works

        uint32_t data2 = temporary_registers->eax;
        uint32_t data3 = temporary_registers->ebx;

        if (data3 == 0 || data3 > 1024) {
            temporary_registers->eax = 0;
            return 0;
        }

        // Allocate memory for the string.
        char* text = (char*)kmalloc(data3 + 1);
        text[data3] = '\0';

        // Read the string from the program.
        bool result = virtual_allocator::read_virtual_memory(scheduler::current_thread->process, reinterpret_cast<void*>(data), data3, (uint8_t*)text);
        if (!result) {
            kfree(text);
            temporary_registers->eax = 0;
            return 0;
        }

        // Print the string.
        video::printf(text, data2);

        // Free the allocated memory.
        kfree(text);

        //speedyshell::printf(reinterpret_cast<char*>(data), (VGA_COLOUR)data2);

        // will be removed.
        //speedyshell::printf("\n");
    } else if (id == 13) {
        // If interface method is not SpeedyShell or graphics mode is pixel.
        if (!isTerminalInterface || speedyshell::pixel_mode) {
            temporary_registers->eax = 0;
            return 0;
        }

        // If another thread is already getting input, return.
        if (speedyshell::input_mode) {
            temporary_registers->eax = 0;
            return 0;
        }

        // Activate input mode.
        speedyshell::input_mode = true;
        speedyshell::allow_typing = true;
        speedyshell::input_thread_id = scheduler::current_thread->id;

        // Suspend thread.
        scheduler::current_thread->state.suspended = true;
        scheduler::current_thread->suspension_details.resume_time = 0;

        // Save thread data.
        scheduler::manual_context_switch_return();

        // Reset inputs.
        speedyshell::clear_buffer();

        // Print cursor.
        video::printf(" ", VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);

        // Saving resources by not re-scheduling
        // as it will be scheduled manually.

        return 1;
    } else if (id == 14) {
        // Hardware random int.
        // Will be brought back in the future.
        temporary_registers->eax = 0;
    } else if (id == 15) {
        uint32_t data2 = temporary_registers->eax;
        uint32_t data3 = temporary_registers->ebx;

        // Create the thread.
        Thread* thread = 
            scheduler::create_thread(
                scheduler::current_thread->process,
                reinterpret_cast<void(*)()>(data), 
                reinterpret_cast<void*>(data2)
            );

        // Return the thread ID.
        temporary_registers->eax = thread->id;
    } else if (id == 16) {
        uint32_t data2 = temporary_registers->eax;

        // If the thread ID is 0, the currently running thread will be killed.
        if (data == 0 || data == scheduler::current_thread->id) {
            // Kill the thread.
            scheduler::kill_thread(scheduler::current_thread, data2);

            // Clear current thread.
            scheduler::current_thread = nullptr;
            scheduler::temporary_registers = &scheduler::placeholder_registers;

            return 1;
        }

        // Check if the thread exists.
        if (!scheduler::thread_list->exists(data)) {
            temporary_registers->eax = 0;
            return 0;
        }

        Thread* thread = scheduler::thread_list->fetch(data);

        // Kill the thread.
        scheduler::kill_thread(thread, data2);

        // Return success status.
        temporary_registers->eax = 1;
    } else if (id == 17) {
        // If the thread ID is 0, the currently running thread will be parked.
        if (data == 0 || data == scheduler::current_thread->id) {
            // Park the thread.
            scheduler::current_thread->state.parked = true;

            // Save data.
            scheduler::manual_context_switch_return();

            return 1;
        }

        // Check if the thread exists.
        if (!scheduler::thread_list->exists(data)) {
            temporary_registers->eax = 0;
            return 0;
        }

        // Park the thread.
        scheduler::thread_list->fetch(data)->state.parked = true;

        // Return success status.
        temporary_registers->eax = 1;
    } else if (id == 18) {
        // Check if the thread exists.
        if (!scheduler::thread_list->exists(data)) {
            temporary_registers->eax = 0;
            return 0;
        }

        // Unpark the thread.
        scheduler::thread_list->fetch(data)->state.parked = false;

        // Return success status.
        temporary_registers->eax = 1;
    } else if (id == 19) {
        // If graphics are already upgraded, return.
        if (scheduler::current_thread->process->paging.pixel_mapping_address != 0) {
            temporary_registers->eax = 0;
            return 0;
        }

        // If the program is a shell executed program.
        if (speedyshell::running_process_id == scheduler::current_thread->process->id) {
            // Save current shell contents.
            //video::savescr();

            // Clear the screen.
            video::clearscr();

            // Update state.
            speedyshell::pixel_mode = true;
        }

        // Allocate and map virtual space for GPU memory.
        uint32_t* video_address_page = reinterpret_cast<uint32_t*>((reinterpret_cast<uint32_t>(graphics::video_address) / 4096) * 4096);
        uint32_t size = ((graphics::resolution_width * graphics::resolution_height) / 4096) + 1;
        uint32_t address_index = virtual_allocator::find_free_virtual_pages(scheduler::current_thread->process, size);
        for (uint32_t i = 0; i < size; i++) {
            PageEntry* page = virtual_allocator::fetch_page_index(scheduler::current_thread->process, address_index + i);
            page->Present = true;
            page->Address = paging::address_to_pi(video_address_page) + i;
            page->UserSupervisor = true;
            page->ReadWrite = true;
        }
        
        // Add offset into the page since buffer is not page aligned usually.
        uint32_t map_address = reinterpret_cast<uint32_t>(paging::pi_to_address(address_index)) + (reinterpret_cast<uint32_t>(graphics::video_address) % 4096);

        scheduler::current_thread->process->paging.pixel_mapping_address = address_index;

        // Return mapped address.
        temporary_registers->eax = reinterpret_cast<uint32_t>(paging::pi_to_address(map_address));
    } else if (id == 20) {
        // Save data and return.
        scheduler::manual_context_switch_return();
        return 1;
    } else if (id == 21) {
        // TODO - check if count is too high.
        uint32_t data2 = temporary_registers->eax;
        uint32_t data3 = temporary_registers->ebx;

        // If count is 0.
        if (data2 == 0) {
            temporary_registers->eax = 0;
            return 0;
        }

        uint32_t page_index_start;

        // If program requests specific address.
        if (data != 0) {
            // If address is not page aligned or available.
            if ((data % 4096) != 0 || !virtual_allocator::preferred_virtual_pages_free(scheduler::current_thread->process, paging::address_to_pi((void*)data), data2)) {
                temporary_registers->eax = 0;
                return 0;
            }

            page_index_start = paging::address_to_pi((void*)data);
        } else page_index_start = virtual_allocator::find_free_virtual_pages(scheduler::current_thread->process, data2);

        // TODO - add error checking
        // TODO - check if physical page allocation failed
        // TODO - optimise by having a skeleton page and simply replacing the changing values
        for (uint32_t i = 0; i < data2; i++) {
            PageEntry* page = virtual_allocator::fetch_page_index(scheduler::current_thread->process, page_index_start + i);
            page->Present = (data3 & SyscallPageAllocationFlag::ALLOCATE_IMMEDIATELY) != 0;
            page->Address = page->Present ? paging::address_to_pi(physical_allocator::alloc_physical_page(1, true)) : 0;
            page->UserSupervisor = true;
            page->ReadWrite = true;
            page->KernelFlags = PageFlags::PROCESS_OWNED;

            // Additional page flags.
            if ((data3 & SyscallPageAllocationFlag::ALLOCATE_ON_ACCESS) != 0) page->KernelFlags |= PageFlags::DYNAMIC_PAGE_ALLOCATION;
        }

        // Return the start of the address.
        temporary_registers->eax = reinterpret_cast<uint32_t>(paging::pi_to_address(page_index_start));
    } else if (id == 22) {
        // Check if the address is page aligned or is protected.
        if (data % 4096 != 0) {
            // kill process
        } else if (data < 104857600) {
            // kill process
        }

        uint32_t addr = paging::address_to_pi(reinterpret_cast<void*>(data));

        // TODO - needs to be implemented
    } else if (id == 23) {
        uint32_t data2 = temporary_registers->eax;
        uint32_t data3 = temporary_registers->ebx;

        if (!scheduler::datasink::active_sinks.exists(data)) {
            temporary_registers->eax = 0;
            return false;
        }

        SteadyDataSink* datasink = scheduler::datasink::active_sinks.fetch(data);
        
        if (
            !datasink->permissions.exists(scheduler::current_thread->process->id) ||
            !datasink->permissions.fetch(scheduler::current_thread->process->id).write
        ) {
            temporary_registers->eax = 0;
            return false;
        }

        if (data3 == 0 || data3 > 2048 || datasink->fragments.get_size() > 200) {
            temporary_registers->eax = 0;
            return false;
        }

        // Allocate temporary buffer.
        uint8_t* buffer = (uint8_t*)kmalloc(data3);

        // Read virtual memory into program.
        bool vm_read_status = virtual_allocator::read_virtual_memory(scheduler::current_thread->process, reinterpret_cast<void*>(data2), data3, buffer, false);
        bool fragment_write_status = datasink->append_data(buffer, data3, SteadyDataSink::AppendType::TRANSFER_BUFFER_OWNERSHIP);
        if (!vm_read_status || !fragment_write_status) {
            kfree(buffer);
            temporary_registers->eax = 0;
            return false;
        }

        temporary_registers->eax = 1;
    }

    return 0;
}

// On system call (called from external program).
extern "C" uint32_t __attribute__((fastcall)) handle_system_call() {
    // Switch to kernel directory.
    paging::switch_directory(paging::kernel_page_directory);

    uint32_t result = handle_system_call_hl();
    if (result == 0) {
        // Switch back to user paging.
        paging::switch_directory(scheduler::current_thread->process->paging.directories);
    }

    return result;
}