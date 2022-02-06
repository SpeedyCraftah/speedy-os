#include "syscalls.h"

#include "events.h"
#include "scheduler.h"

#include "../software/system/speedyshell/main.h"
#include "structures/events.h"
#include "structures/thread.h"

#include "../kernel.h"

//temp
#include "../misc/conversions.h"

extern "C" Registers temporary_registers;

// ID 0 = Do nothing (nop).
// ID 1 = Query kernel for data.
//      - 0 = Elapsed time since startup.
//      - 1 = Process ID of process.
//      - 2 = Thread ID of thread.
// ID 2 = End process request (with status code). 
// ID 3 = Temporarily suspend a thread with specific conditions.
// ID 4 = Malloc x bytes of memory.
// ID 5 = Free segment of memory.
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

// On system call (called from external program).
extern "C" uint32_t __attribute__((fastcall)) handle_system_call(uint32_t id, uint32_t data) {
    if (id == 1) {
        if (data == 0) temporary_registers.eax = scheduler::elapsed_ms;
        else if (data == 1) temporary_registers.eax = scheduler::current_thread->process->id;
        else if (data == 2) temporary_registers.eax = scheduler::current_thread->id;

        //video::printf(conversions::s_int_to_char(temporary_registers.esp));
        //video::printf(" ");
    } else if (id == 2) {
        scheduler::kill_process(scheduler::current_thread->process, data);
        scheduler::current_thread = nullptr;

        return 1;
    } else if (id == 3) {
        // May add conditional suspensions.
        // May add process wide suspensions.

        // Apply suspension.
        scheduler::current_thread->state.suspended = true;
        scheduler::current_thread->suspension_details.resume_time =
            data == 0 ? 1 : scheduler::elapsed_ms + data;

        // Save state and reschedule.
        scheduler::manual_context_switch_return();

        return 1;
    // Todo - make memory allocations thread-wide.
    } else if (id == 4) {
        uint8_t* memory_ptr = (uint8_t*)heap::malloc(data, true, false, scheduler::current_thread->process->id);
        temporary_registers.eax = reinterpret_cast<uint32_t>(memory_ptr);
    } else if (id == 5) {
        void* memory_ptr = reinterpret_cast<void*>(data);
        uint32_t deal_result = heap::free(memory_ptr);
        temporary_registers.eax = deal_result;
    } else if (id == 6) {
        // If there is no event currently running, return.
        if (!scheduler::current_thread->state.processing_event) return 0;

        // Clear event flag.
        scheduler::current_thread->state.processing_event = false;

        // Restore registers.
        scheduler::current_thread->registers = scheduler::current_thread->backup_registers;

        // Schedule the thread.
        scheduler::thread_execution_queue->push(scheduler::current_thread);

        // Clear current thread.
        scheduler::current_thread = nullptr;

        return 1;
    } else if (id == 7) {
        // May be moved to a function in events.cpp.
        uint32_t data2 = temporary_registers.eax;
        uint32_t data3 = temporary_registers.ebx;

        // Check if process exists.
        if (!scheduler::process_list->exists(data)) {
            temporary_registers.eax = 0;
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
        temporary_registers.eax = 1;
    } else if (id == 8) {
        uint32_t data2 = temporary_registers.eax;

        scheduler::events::emit_event(scheduler::current_thread->process, data, data2);
    } else if (id == 9) {
        // Future: end event if processing event.
        uint32_t data2 = temporary_registers.eax;

        // Check if execution policy is valid.
        if (data < 0 || data > 2) return 0;

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
            temporary_registers.eax = 0;

            return 0;
        }

        Process* process = scheduler::process_name_list->fetch((char*)data);
        temporary_registers.eax = process->id;
    } else if (id == 11) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            temporary_registers.eax = 0;
            return 0;
        }

        // To-do: copy text instead of direct pointer.
        char* input = speedyshell::text_buffer;
        temporary_registers.eax = reinterpret_cast<uint32_t>(input);
    } else if (id == 12) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            temporary_registers.eax = 0;
            return 0;
        }

        uint32_t data2 = temporary_registers.eax;

        speedyshell::printf(reinterpret_cast<char*>(data), (VGA_COLOUR)data2);

        // will be removed.
        speedyshell::printf("\n");
    } else if (id == 13) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            temporary_registers.eax = 0;
            return 0;
        }

        // If another thread is already getting input, return.
        if (speedyshell::input_mode) {
            temporary_registers.eax = 0;
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
        temporary_registers.eax = 0;
    } else if (id == 15) {
        // Create the thread.
        Thread* thread = 
            scheduler::create_thread(scheduler::current_thread->process, reinterpret_cast<void(*)()>(data));

        // Return the thread ID.
        temporary_registers.eax = thread->id;
    } else if (id == 16) {
        uint32_t data2 = temporary_registers.eax;

        // If the thread ID is 0, the currently running thread will be killed.
        if (data == 0 || data == scheduler::current_thread->id) {
            // Kill the thread.
            scheduler::kill_thread(scheduler::current_thread, data2);

            // Clear current thread.
            scheduler::current_thread = nullptr;

            return 1;
        }

        // Check if the thread exists.
        if (!scheduler::thread_list->exists(data)) {
            temporary_registers.eax = 0;
            return 0;
        }

        Thread* thread = scheduler::thread_list->fetch(data);

        // Kill the thread.
        scheduler::kill_thread(thread, data2);

        // Return success status.
        temporary_registers.eax = 1;
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
            temporary_registers.eax = 0;
            return 0;
        }

        // Park the thread.
        scheduler::thread_list->fetch(data)->state.parked = true;

        // Return success status.
        temporary_registers.eax = 1;
    } else if (id == 18) {
        // Check if the thread exists.
        if (!scheduler::thread_list->exists(data)) {
            temporary_registers.eax = 0;
            return 0;
        }

        // Park the thread.
        scheduler::thread_list->fetch(data)->state.parked = false;

        // Return success status.
        temporary_registers.eax = 1;
    }

    return 0;
}