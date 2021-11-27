#include "syscalls.h"
#include "events.h"
#include "scheduler.h"
#include "../kernel.h"
#include "../software/system/speedyshell/main.h"
#include "structures/state.h"
#include "../misc/random.h"

extern "C" Registers TEMP_REGISTERS;

// ID 0 = Do nothing (nop).
// ID 1 = Query kernel for data.
//      - 0 = Elapsed time since startup.
//      - 1 = Process ID of process.
// ID 2 = End process request (with status code). 
// ID 3 = Temporarily suspend a process with specific conditions.
// ID 4 = Malloc x bytes of memory.
// ID 5 = Free segment of memory.
// ID 6 = End of event call.
// ID 7 = Register event for process.
// ID 8 = Emit event for registered processes.
// ID 9 = Change program status.
// ID 10 = Find process ID of string.
// ID 14 = Get hardware-supported random 32-bit number.

// SpeedyShell Only
// ID 11 = Query SpeedyShell for data.
//       - 0 = Get whole command string.
// ID 12 = Print text to terminal with colour.
// ID 13 = Get input from user.

// Return 0 = Resume program execution.
// Return 1 = Halt and wait for next interrupt.

// Param Order: ECX (ID), EDX, EAX, EBX, Stack...

// On system call (called from external program).
extern "C" uint32_t __attribute__((fastcall)) on_system_call(uint32_t id, uint32_t data) {
    if (id == 1) {
        if (data == 0) TEMP_REGISTERS.eax = scheduler::elapsed_ms;
        else if (data == 1) TEMP_REGISTERS.eax = scheduler::current_process;
    } else if (id == 2) {
        scheduler::end_process(scheduler::current_process, data);

        scheduler::current_process = 0;

        return 1;
    } else if (id == 3) {
        uint32_t data2 = TEMP_REGISTERS.eax;

        if (
            data2 != SuspensionType::FULL &&
            data2 != SuspensionType::EVENTS_ONLY
        ) return 0;
        
        Process* process = scheduler::get_process_list()->fetch(scheduler::current_process);

        // Save process registers.
        if (scheduler::event_running) {
            // Events currently only support full program suspensions.
            if (data2 != SuspensionType::FULL) return 0;

            process->event_receiver.registers = TEMP_REGISTERS;
            process->event_receiver.queue->get_at(0).overtime = true;
        } else process->registers = TEMP_REGISTERS;

        process->current_status = TaskStatus::SUSPENDED;
        process->suspended_type = (SuspensionType)data2;
        process->suspended_until = data == 0 ? 0 : (scheduler::elapsed_ms + data);

        scheduler::get_process_queue()->push(process->id);

        scheduler::current_process = 0;

        return 1;
    } else if (id == 4) {
        uint8_t* memory_ptr = (uint8_t*)heap::malloc(data, true, false, scheduler::current_process);
        TEMP_REGISTERS.eax = reinterpret_cast<uint32_t>(memory_ptr);
    } else if (id == 5) {
        void* memory_ptr = reinterpret_cast<void*>(data);
        uint32_t deal_result = heap::free(memory_ptr);
        TEMP_REGISTERS.eax = deal_result;
    } else if (id == 6) {
        // If there is no event currently running.
        if (!scheduler::event_running) return 0;
        
        Process* process = scheduler::get_process_list()->fetch(scheduler::current_process);

        // Remove event from queue (mark as done).
        process->event_receiver.queue->shift();

        // Restore event receiver registers.
        process->event_receiver.registers = Registers();
        process->event_receiver.registers.esp = 
            reinterpret_cast<uint32_t>(process->event_receiver.stack_base) + (1 * 1024);

        // Exit from event and prevent scheduler from saving state on return.
        scheduler::current_process = 0;
        scheduler::event_running = false;

        // Schedule execution.
        scheduler::get_process_queue()->push(process->id);

        return 1;
    } else if (id == 7) {
        uint32_t data2 = TEMP_REGISTERS.eax;
        uint32_t data3 = TEMP_REGISTERS.ebx;

        bool result = scheduler::events::add_event_listener(
            scheduler::current_process,
            data, 
            data2, 
            data3
        );

        TEMP_REGISTERS.eax = result;
    } else if (id == 8) {
        uint32_t data2 = TEMP_REGISTERS.eax;

        scheduler::events::emit_event(scheduler::current_process, data, data2);
    } else if (id == 9) {
        // Only these are supported. Suspensions are managed via another system call.
        if (
            data != TaskStatus::RUNNING &&
            data != TaskStatus::RUNNING_BUSY &&
            data != TaskStatus::RUNNING_WAITING_FOR_DATA
        ) return 0;

        Process* process = scheduler::get_process_list()->fetch(scheduler::current_process);
        
        if (process->current_status != TaskStatus::SUSPENDED) process->current_status = 
            (TaskStatus)data;

        process->main_status = (TaskStatus)data;

        if (data == TaskStatus::RUNNING_BUSY && scheduler::event_running) {
            // Dump event registers.
            process->event_receiver.registers = TEMP_REGISTERS;
            process->event_receiver.queue->get_at(0).overtime = true;

            scheduler::get_process_queue()->push(process->id);
            scheduler::current_process = 0;

            return 1;
        }

        else if (data == TaskStatus::RUNNING_WAITING_FOR_DATA && !scheduler::event_running) {
            // Dump registers.
            process->registers = TEMP_REGISTERS;

            scheduler::get_process_queue()->push(process->id);
            scheduler::current_process = 0;

            return 1;
        }

        else return 0; 
    } else if (id == 10) {
        if (!scheduler::get_process_list_string()->exists((char*)data)) {
            TEMP_REGISTERS.eax = 0;
            return 0;
        }

        Process* process = scheduler::get_process_list_string()->fetch((char*)data);
        TEMP_REGISTERS.eax = process->id;
    } else if (id == 11) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            TEMP_REGISTERS.eax = 0;
            return 0;
        }

        // To-do: copy text instead of direct pointer.
        char* input = speedyshell::get_text_input();
        TEMP_REGISTERS.eax = reinterpret_cast<uint32_t>(input);
    } else if (id == 12) {
        // If interface method is not SpeedyShell.
        if (!isTerminalInterface) {
            TEMP_REGISTERS.eax = 0;
            return 0;
        }

        uint32_t data2 = TEMP_REGISTERS.eax;

        speedyshell::printf(reinterpret_cast<char*>(data), (VGA_COLOUR)data2);
    } else if (id == 13) {
        // If interface method is not SpeedyShell.
        // Does not support events at the moment.
        if (!isTerminalInterface || scheduler::event_running) {
            TEMP_REGISTERS.eax = 0;
            return 0;
        }

        Process* process = scheduler::get_process_list()->fetch(scheduler::current_process);

        // Activate input mode.
        speedyshell::input_mode = true;

        // Suspend program.
        process->current_status = TaskStatus::SUSPENDED;
        process->suspended_type = SuspensionType::FULL;
        process->suspended_until = 0;

        // Save registers.
        process->registers = TEMP_REGISTERS;

        scheduler::current_process = 0;

        // Reset inputs.
        speedyshell::clear_input();

        // Print cursor.
        video::printf(" ", VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);

        // Saving resources by not re-scheduling
        // as it will be scheduled manually.

        return 1;
    } else if (id == 14) {
        TEMP_REGISTERS.eax = random::next();
    }

    return 0;
}