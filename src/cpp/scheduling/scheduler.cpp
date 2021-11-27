#include "scheduler.h"

#include "../abstractions/io_port.h"
#include "../heap/allocator.h"
#include "../abstractions/cpu.h"
#include "../chips/pic.h"
#include "events.h"
#include "../misc/conversions.h"

#include <stdint.h>

// Keep track of elapsed time.
extern "C" uint32_t elapsed_ms = 0;

// 0 = Scheduler idling.
extern "C" uint32_t current_process = 0;
static uint32_t next_process_id = 1;

extern "C" bool event_running = false;

// Allows ASM and C++ to access registers.
extern "C" Registers TEMP_REGISTERS = Registers();

// Assembly functions.
extern "C" void scheduler_switch_to_task();
extern "C" void scheduler_sleep();

extern "C" uint8_t kernel_stack;

static structures::map<Process*>* process_list;
static structures::map<Process*>* process_list_string;
static structures::flexible_array<uint32_t>* process_queue;

static uint32_t scheduler_process_id;

namespace scheduler {
    void initialise() {
        // Initialise queue & process map.
        process_list = new structures::map<Process*>(15);
        process_list_string = new structures::map<Process*>(15);
        process_queue = new structures::flexible_array<uint32_t>(15);
        
        // Start event process.
        scheduler_process_id = start_process(
            structures::string("Scheduler"),
            0,
            TaskStatus::RUNNING_WAITING_FOR_DATA,
            ProcessFlag::SYSTEM_DRIVER, 
            false, 
            true
        );
    }

    void __attribute__((fastcall)) switch_to_task(uint32_t process_id) {
        Process* process = process_list->fetch(process_id);

        TEMP_REGISTERS = process->registers;
        current_process = process->id;

        event_running = false;

        return scheduler_switch_to_task();
    }

    structures::map<Process*>* get_process_list() {
        return process_list;
    }

    structures::map<Process*>* get_process_list_string() {
        return process_list_string;
    }

    structures::flexible_array<uint32_t>* get_process_queue() {
        return process_queue;
    }
    // allocator.
    void end_process(uint32_t process_id, uint32_t code) {
        Process* process = process_list->fetch(process_id);

        // Deallocate stack.
        heap::free(process->stack_base);

        if (process->event_emitter.supported) {
            delete process->event_emitter.subscribed;
        }

        if (process->event_receiver.supported) {
            delete process->event_receiver.queue;

            // Deallocate stack.
            heap::free(process->event_receiver.stack_base);
        }

        // Emit event.
        scheduler::events::emit_event(scheduler_process_id, 2, process->id);

        process_list_string->remove(process->name);

        // Free heap used by the process.
        heap::free_by_process_id(process_id);

        // Remove process.
        process_list->remove(process->id);

        // Deallocate process name.
        delete process->name;

        // Deallocate process object.
        delete process;
    }

    uint32_t start_process(structures::string name, void(*entry)(), TaskStatus status, uint32_t flags, bool event_receiver_support, bool event_emitter_support) {
        // If name is too long.
        if (name.length() > 20) return 0;

        // If name is already taken.
        if (process_list_string->exists(name)) {
            return 0;
        }

        // Set process attributes.
        Process* new_process = new Process;
        new_process->id = next_process_id++;
        new_process->name = name.char_copy().norm();
        new_process->priority = TaskPriority::NORMAL;
        new_process->current_status = 
            (flags & ProcessFlag::SYSTEM_DRIVER) == 0 ? status : TaskStatus::RUNNING_WAITING_FOR_DATA;
        new_process->main_status = new_process->current_status;
        new_process->flags = flags;
        new_process->total_cpu_time = 0;

        // If process will support receiving events.
        if (event_receiver_support) {
            new_process->event_receiver.supported = true;
            new_process->event_receiver.queue = new structures::flexible_array<TaskEvent>();

            // Create stack for event handler (1kb).
            uint8_t* stack = (uint8_t*)heap::malloc(1 * 1024);
            new_process->event_receiver.stack_base = (void*)stack;

            // Setup separate registers for event handler.
            Registers new_registers;
            new_registers.esp = reinterpret_cast<uint32_t>(stack) + (1 * 1024);

            new_process->event_receiver.registers = new_registers;
        } else new_process->event_receiver.supported = false;

        // If process will support dispatching events.
        if (event_emitter_support) {
            new_process->event_emitter.supported = true;
            new_process->event_emitter.subscribed = new structures::flexible_array<TaskEventSubscription>();
        } else new_process->event_emitter.supported = false;

        // If process is a driver, it does not need registers or a stack.
        if ((flags & ProcessFlag::SYSTEM_DRIVER) == 0) {
            // Create a stack for the new process (2kb).
            uint8_t* stack = (uint8_t*)heap::malloc(2 * 1024);
            new_process->stack_base = (void*)stack;

            // Setup registers (only load eflags due to bugs).
            Registers new_registers;
            new_registers.eflags = TEMP_REGISTERS.eflags;

            // Point to the top of the stack.
            new_registers.esp = reinterpret_cast<uint32_t>(stack) + (2 * 1024);
            
            // Set entry location.
            new_registers.eip = (uint32_t)entry;

            // Save registers.
            new_process->registers = new_registers;
        }

        // If process is either not a driver or a driver with an event receiver.
        // Schedule execution.
        if ((flags & ProcessFlag::SYSTEM_DRIVER) == 0 && event_receiver_support) {
            process_queue->push(new_process->id);
        } else process_queue->push(new_process->id);

        // Add process to registry.
        process_list->set(new_process->id, new_process);
        process_list_string->set(new_process->name, new_process);

        // Emit event.
        scheduler::events::emit_event(scheduler_process_id, 1, new_process->id);
        
        return new_process->id;
    }

    // On PIT tick (called from assembly).
    extern "C" void on_scheduler_timer_interrupt_main() {
        // Add time to elapsed time.
        elapsed_ms += 2;

        // If process finished running.
        if (current_process != 0) {
            Process* process = process_list->fetch(current_process);

            // If previous execution was an event.
            // Save process registers.
            if (event_running) {
                process->event_receiver.registers = TEMP_REGISTERS;
                process->event_receiver.queue->get_at(0).overtime = true;
            } else {
                process->registers = TEMP_REGISTERS;
            }

            // Add CPU time.
            process->total_cpu_time += 2;

            // Schedule process for execution.
            process_queue->push(current_process);
        }

        uint32_t iter_count = 0;

        while (true) {
            iter_count++;

            // If scheduler has nothing to do.
            if (process_queue->get_size() == 0 || iter_count > process_queue->get_size()) {
                current_process = 0;

                // Halt the CPU and wait until next interrupt.
                return scheduler_sleep();
            }

            uint32_t id = process_queue->shift();

            // If process no longer exists.
            if (!process_list->exists(id)) {
                continue;
            }

            // If CPU has a process to execute.
            Process* process = process_list->fetch(id);

            if (process->current_status == TaskStatus::SUSPENDED) {
                // If suspension period expired.
                if (process->suspended_until != 0 && elapsed_ms >= process->suspended_until) {
                    // Unsuspend process.
                    process->current_status = process->main_status;
                } else {
                    // Check next time.
                    process_queue->push(id);
                    
                    // If suspension is event only and events are present, it can run.
                    if (
                        process->suspended_type != SuspensionType::EVENTS_ONLY ||
                        !process->event_receiver.supported ||
                        process->event_receiver.queue->get_size() == 0
                    ) continue;
                }
            }

            // If process supports events and has a supported state.
            if (
                process->event_receiver.supported &&
                (process->main_status == TaskStatus::RUNNING 
                || process->main_status == TaskStatus::RUNNING_WAITING_FOR_DATA)
            ) {
                // If process has events pending.
                if (process->event_receiver.queue->get_size() != 0) {
                    // Run event.
                    TaskEvent event = process->event_receiver.queue->get_at(0);

                    TEMP_REGISTERS = process->event_receiver.registers;

                    current_process = process->id;

                    // If first time processing event.
                    if (!event.overtime) {
                        // Set handler pointer.
                        TEMP_REGISTERS.eip = reinterpret_cast<uint32_t>(event.handler_ptr);

                        // Provide event params (function must be fastcall).
                        TEMP_REGISTERS.ecx = event.event_id;
                        TEMP_REGISTERS.edx = event.event_data;
                    }

                    event_running = true;

                    return scheduler_switch_to_task();
                }
            }

            // If process still has nothing to do and is only waiting for events.
            if (process->current_status == TaskStatus::RUNNING_WAITING_FOR_DATA) {
                process_queue->push(id);
                continue;
            }

            // Switch to the process.
            return switch_to_task(id);
        }

        return scheduler_sleep();
    }
}
