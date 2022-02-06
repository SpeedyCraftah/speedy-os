#include "scheduler.h"

#include "events.h"
#include "stdint.h"

#include "../structures/flex_array.h"
#include "../structures/map.h"

#include "structures/events.h"
#include "structures/process.h"
#include "structures/thread.h"
#include "events.h"

// remove after
#include "../software/system/speedyshell/main.h"
#include "../misc/conversions.h"
#include "../software/include/sys.h"

extern "C" Registers temporary_registers = Registers();

// Import the assembly functions.
extern "C" void scheduler_sleep();
extern "C" void scheduler_execute();


namespace scheduler {
    uint32_t elapsed_ms = 0;

    Thread* current_thread = nullptr;

    uint32_t next_thread_id = 1;
    uint32_t next_process_id = 1;

    structures::map<Thread*>* thread_list;
    structures::map<Process*>* process_list;
    structures::linked_array<ThreadEvent>* thread_event_queue;
    structures::linked_array<Thread*>* thread_execution_queue;
    structures::map<Process*>* process_name_list;

    // Iterators.
    structures::linked_array<ThreadEvent>::iterator thread_event_iterator;
    structures::linked_array<Thread*>::iterator thread_execution_iterator;

    Process* scheduler_event_process;

    void initialise() {
        // Create the structures.
        thread_list = new structures::map<Thread*>(20);
        process_list = new structures::map<Process*>(10);
        thread_event_queue = new structures::linked_array<ThreadEvent>(15);
        thread_execution_queue = new structures::linked_array<Thread*>(15);
        process_name_list = new structures::map<Process*>(10);

        // Create the iterators.
        thread_event_iterator = thread_event_queue->create_iterator();
        thread_execution_iterator = thread_execution_queue->create_iterator();

        ProcessFlags flags;
        flags.system_process = true;
        flags.virtual_process = true;

        scheduler_event_process = create_process("Scheduler Events", 0, flags);
    }

    extern "C" void handle_context_switch() {
        // If a thread was previously running.
        if (current_thread != nullptr) {
            // Save the registers.
            current_thread->registers = temporary_registers;

            // Reschedule the thread.
            thread_execution_queue->push(current_thread);

            // Clear current thread.
            current_thread = nullptr;
        }

        // Reset iterator.
        thread_event_iterator.reset();

        // Check the event queue for outstanding events (to be improved).
        while (thread_event_iterator.hasNext()) {
            ThreadEvent event = thread_event_iterator.next();

            // If the thread is already running an event, continue.
            if (event.thread->state.processing_event) continue;

            // If the threads execution policy blocks events, remove.
            if (event.thread->execution_policy == ThreadExecutionPolicy::BUSY) {
                thread_event_iterator.remove();
                continue;
            }

            // If the thread is parked, unpark the thread.
            event.thread->state.parked = false;

            // Backup the current threads registers.
            event.thread->backup_registers = event.thread->registers;

            // Set the EIP to the event handler.
            event.thread->registers.eip = reinterpret_cast<uint32_t>(event.handler);

            // Mark the thread as busy to queue future events.
            event.thread->state.processing_event = true;

            // Create a stack frame.
            uint32_t* stack = 
                reinterpret_cast<uint32_t*>(event.thread->registers.esp) - 3;

            // Push the event data.
            *(stack + 1) = event.event_id;
            *(stack + 2) = event.event_data;

            event.thread->registers.esp = reinterpret_cast<uint32_t>(stack);

            // Remove the event from the queue.
            thread_event_iterator.remove();
        }

        // Check if any thread needs executing.
        for (uint32_t i = 0; i < thread_execution_queue->get_size(); i++) {            
            // Fetch thread from queue.
            Thread* thread = thread_execution_queue->shift();

            // If thread only reacts on events, continue.
            if (thread->execution_policy == ThreadExecutionPolicy::EVENT_ONLY && !thread->state.processing_event) {
                thread_execution_queue->push(thread);
                continue;
            }

            // If the process is suspended, continue.
            if (thread->process->state.suspended) {
                thread_execution_queue->push(thread);
                continue;
            }

            // If the thread is parked, continue.
            if (thread->state.parked) {
                thread_execution_queue->push(thread);
                continue;
            }

            // If the thread is suspended and the suspension is temporary.
            if (thread->state.suspended) {
                if (thread->suspension_details.resume_time != 0) {
                    // If the suspension time has passed, resume the thread.
                    if (thread->suspension_details.resume_time <= elapsed_ms) {
                        thread->state.suspended = false;
                        thread->suspension_details.resume_time = 0;
                    } else {
                        // Reschedule.
                        thread_execution_queue->push(thread);
                        continue;
                    }
                } else {
                    // Suspension is permanent, reschedule.
                    thread_execution_queue->push(thread);
                    continue;
                }
            }

            // Execute the thread.

            // Set the current thread.
            current_thread = thread;

            // Load threads registers to temporary storage.
            temporary_registers = thread->registers;

            // Switch to assembly side of scheduler to begin execution.
            return scheduler_execute();
        }

        // If there is nothing available to execute, sleep.
        return scheduler_sleep();
    }

    extern "C" void handle_timer_tick() {
        // Increment the elapsed time.
        elapsed_ms += 2;

        // Handle context switch side of the timer.
        return handle_context_switch();
    }

    Process* create_process(char* name, void (*entry)(), ProcessFlags flags) {
        // Create the process.
        Process* process = new Process;
        process->id = next_process_id++;
        process->name = structures::string(name).char_copy().norm();
        process->flags = flags;
        process->threads = new structures::linked_array<Thread*>(8);
        process->hooked_threads = new structures::linked_array<ThreadEventListener>(6);

        // Add the process to the map.
        process_list->set(process->id, process);
        process_name_list->set(process->name, process);
        
        // If the process is not a virtual process.
        // Create main thread.
        // Virtual processes have no running thread.
        if (!flags.virtual_process) {
            Thread* thread = new Thread;
            thread->id = next_thread_id++;
            thread->process = process;
            thread->flags.main_thread = true;
            thread->registers.eip = reinterpret_cast<uint32_t>(entry);
            
            // Create an 8KB stack.
            thread->stack = heap::malloc(8192);

            // Point to top of stack.
            thread->registers.esp = reinterpret_cast<uint32_t>(thread->stack) + 8192;

            // Add the thread to the map.
            thread_list->set(thread->id, thread);
            
            // Add thread to process.
            process->threads->push(thread);

            // Schedule the thread.
            thread_execution_queue->push(thread);
        }

        // Emit process create event.
        scheduler::events::emit_event(scheduler_event_process, 1, process->id);

        // Return the process.
        return process;
    }

    Thread* create_thread(Process* process, void (*entry)()) {
        Thread* thread = new Thread;
        thread->id = next_thread_id++;
        thread->process = process;
        thread->registers.eip = reinterpret_cast<uint32_t>(entry);
        
        // Create an 8KB stack.
        thread->stack = heap::malloc(8192);

        // Point to top of stack.
        thread->registers.esp = reinterpret_cast<uint32_t>(thread->stack) + 8192;

        // Add the thread to the map.
        thread_list->set(thread->id, thread);

        // Add thread to process.
        process->threads->push(thread);

        // Schedule the thread.
        thread_execution_queue->push(thread);

        return thread;
    }

    void kill_thread(Thread* thread, uint32_t code) {
        // If the thread is the main thread for the process, kill the process.
        if (thread->flags.main_thread) {
            return kill_process(thread->process, code);
        }

        // Iterate through thread process list and remove.
        auto thread_process_iterator = thread->process->threads->create_iterator();

        while (thread_process_iterator.hasNext()) {
            Thread* p_thread = thread_process_iterator.next();

            if (p_thread == thread) thread_process_iterator.remove();
        }

        // Reset iterator.
        thread_event_iterator.reset();

        // Iterate through threads in event queue and remove.
        while (thread_event_iterator.hasNext()) {
            ThreadEvent event = thread_event_iterator.next();

            // If process owns the thread, remove.
            if (event.thread == thread) thread_event_iterator.remove();
        }

        // Reset iterator.
        thread_execution_iterator.reset();

        // Iterate through execution queue and remove.
        while (thread_execution_iterator.hasNext()) {
            Thread* q_thread = thread_execution_iterator.next();

            // If process owns the thread, remove.
            if (q_thread == thread) thread_execution_iterator.remove();
        }

        // Deallocate and remove.
        heap::free(thread->stack);
        delete thread;
    }

    void kill_process(Process* process, uint32_t code) {
        // Remove the process from map.
        process_list->remove(process->id);
        process_name_list->remove(process->name);

        // Reset iterator.
        thread_event_iterator.reset();

        // Iterate through threads in event queue and remove.
        while (thread_event_iterator.hasNext()) {
            ThreadEvent event = thread_event_iterator.next();

            // If process owns the thread, remove.
            if (event.thread->process == process) thread_event_iterator.remove();
        }

        // Reset iterator.
        thread_execution_iterator.reset();

        // Iterate through execution queue and remove.
        while (thread_execution_iterator.hasNext()) {
            Thread* thread = thread_execution_iterator.next();

            // If process owns the thread, remove.
            if (thread->process == process) thread_execution_iterator.remove();
        }

        // Iterate through the processes threads and terminate.
        auto process_threads_iterator = process->threads->create_iterator();

        while (process_threads_iterator.hasNext()) {
            Thread* thread = process_threads_iterator.next();

            thread_list->remove(thread->id);

            // Deallocate objects.
            heap::free(thread->stack);
            heap::free(thread);
        }

        // Emit process end event.
        scheduler::events::emit_event(scheduler_event_process, 2, process->id);

        // Free all memory used by process.
        heap::free_by_process_id(process->id);

        // Free process objects.
        delete process->threads;
        delete process->hooked_threads;
        heap::free(process->name);

        // Remove the process itself.
        delete process;
    }

    void manual_context_switch_return() {
        // Save the registers.
        current_thread->registers = temporary_registers;

        // Schedule the thread.
        thread_execution_queue->push(current_thread);

        // Clear current thread.
        current_thread = nullptr;
    }
}