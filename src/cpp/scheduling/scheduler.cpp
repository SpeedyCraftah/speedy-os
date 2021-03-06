#include "scheduler.h"

#include "events.h"
#include "stdint.h"

#include "../structures/flex_array.h"
#include "../structures/map.h"

#include "structures/events.h"
#include "structures/process.h"
#include "structures/thread.h"
#include "events.h"
#include "../misc/memory.h"

// remove after
#include "../software/system/speedyshell/main.h"
#include "../misc/conversions.h"
#include "../software/include/sys.h"
#include "../chips/pit.h"
#include "../misc/assert.h"

// Temporary registers switched to pointer for performance.
extern "C" Registers placeholder_registers = Registers();
extern "C" Registers* temporary_registers = nullptr;
extern "C" void* temporary_eip = nullptr;

// Import the assembly functions.
extern "C" void scheduler_sleep();
extern "C" void scheduler_execute();
extern "C" void save_fpu_boot_state();

// If defined the scheduler debug runtime tools will be compiled and enabled.
// Also enables asserts in areas where there is performance impact of doing so.
#define SCHEDULER_DEBUG

namespace scheduler {
    uint32_t elapsed_ms = 0;
    uint32_t time_slice_ms = 5;
    uint32_t tsc_tick_rate = 0;
    float scheduler_idle_time_ms = 0;
    float last_scheduler_idle_time = 0;
    float scheduler_running_time_ms = 0;
    float last_scheduler_running_time_ms = 0;
    bool debug_mode = false;
    char fpu_boot_state[108];

    Thread* current_thread = nullptr;
    bool timer_preempt = false;

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
        // Set base FPU state.
        save_fpu_boot_state();

        // Set default register destination.
        temporary_registers = &placeholder_registers;

        // Create the structures.
        thread_list = new structures::map<Thread*>(20);
        process_list = new structures::map<Process*>(10);
        thread_event_queue = new structures::linked_array<ThreadEvent>(15);
        thread_execution_queue = new structures::linked_array<Thread*>(15);
        process_name_list = new structures::map<Process*>(10);

        assert_eq("sch.structs.threads", thread_list->get_capacity(), (uint32_t)20);
        assert_eq("sch.structs.processes", process_list->get_capacity(), (uint32_t)10);
        assert_eq("sch.structs.events", thread_event_queue->get_capacity(), (uint32_t)15);
        assert_eq("sch.structs.execution", thread_execution_queue->get_capacity(), (uint32_t)15);
        assert_eq("sch.structs.procnames", process_name_list->get_capacity(), (uint32_t)10);

        // Create the iterators.
        thread_event_iterator = thread_event_queue->create_iterator();
        thread_execution_iterator = thread_execution_queue->create_iterator();

        ProcessFlags flags;
        flags.system_process = true;
        flags.virtual_process = true;

        scheduler_event_process = create_process("Scheduler Events", 0, flags);

        assert_eq("sch.procs.1.name.exists", process_name_list->exists("Scheduler Events"), true);
        assert_eq("sch.procs.1.id", process_name_list->fetch("Scheduler Events")->id, scheduler_event_process->id);
        assert_eq("sch.procs.1.id.exists", process_list->exists(scheduler_event_process->id), true);
    }

    extern "C" void handle_context_switch() {
        // If a thread was previously running.
        if (current_thread != nullptr) {
            float cpu_time_used =
                timer_preempt ? time_slice_ms : (time_slice_ms - chips::pit::fetch_channel_0_remaining_countdown());
            
            scheduler::scheduler_running_time_ms += cpu_time_used;
            current_thread->process->total_cpu_time += cpu_time_used;

            #ifdef SCHEDULER_DEBUG
                uint32_t old_exec_length = thread_execution_queue->get_size();
            #endif

            // Reschedule the thread.
            thread_execution_queue->push(current_thread);

            #ifdef SCHEDULER_DEBUG
                assert_eq("sch.structs.execution.length", old_exec_length + 1, thread_execution_queue->get_size());
                assert_eq("sch.structs.execution.peek.id", thread_execution_queue->peek()->id, current_thread->id);
            #endif

            // Clear current thread.
            current_thread = nullptr;

            // Set temporary registers back to placeholder.
            temporary_registers = &placeholder_registers;
        }

        // Reset iterator.
        thread_event_iterator.reset();

        // Check the event queue for outstanding events (to be improved).
        while (thread_event_iterator.hasNext()) {
            ThreadEvent event = thread_event_iterator.next();

            #ifdef SCHEDULER_DEBUG
                assert_eq("sch.events.thread.heap", heap::allocated(event.thread), true);
                uint32_t old_ev_length = thread_event_queue->get_size();
            #endif

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

            #ifdef SCHEDULER_DEBUG
                assert_eq("sch.structs.events.length", old_ev_length - 1, thread_event_queue->get_size());
            #endif
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
            temporary_registers = &thread->registers;
            temporary_eip = reinterpret_cast<void*>(thread->registers.eip);

            // Switch to assembly side of scheduler to begin execution.
            return scheduler_execute();
        }

        // If there is nothing available to execute, add sleep time and sleep.
        
        scheduler_idle_time_ms += chips::pit::fetch_channel_0_remaining_countdown();

        // Sleep.
        return scheduler_sleep();
    }

    // Holds the data for timeslices.
    uint32_t timer_ms_timeslice = 0;

    extern "C" void handle_timer_tick() {
        // Set timer preempt state.
        timer_preempt = true;

        // Increment the elapsed time.
        elapsed_ms += time_slice_ms;

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

        assert_eq("sch.procs.name.heap", heap::allocated(process->name), true);
        assert_eq("sch.procs.heap", heap::allocated(process->name), true);

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
            
            // Load the setup FPU state.
            memcpy(thread->registers.fpu_state, fpu_boot_state, sizeof(fpu_boot_state));

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

    Thread* create_thread(Process* process, void (*entry)(), void* capture) {
        Thread* thread = new Thread;
        thread->id = next_thread_id++;
        thread->process = process;
        thread->registers.eip = reinterpret_cast<uint32_t>(entry);

        assert_eq("sch.procs.threads.new.heap", heap::allocated(thread), true);
        assert_eq("sch.procs.threads.new.proc.heap", heap::allocated(process), true);

        // Load the setup FPU state.
        memcpy(thread->registers.fpu_state, fpu_boot_state, sizeof(fpu_boot_state));
        
        // Create an 8KB stack.
        thread->stack = heap::malloc(8192);

        // Point to top of stack.
        thread->registers.esp = reinterpret_cast<uint32_t>(thread->stack) + 8192;

        // Place the capture on the stack.
        thread->registers.esp -= sizeof(uint32_t) * 2;
        uint32_t* stack = reinterpret_cast<uint32_t*>(thread->registers.esp);
        stack[1] = reinterpret_cast<uint32_t>(capture);
        
        // Add the thread to the map.
        thread_list->set(thread->id, thread);

        // Add thread to process.
        process->threads->push(thread);

        assert_eq("sch.procs.threads.list.new", thread_list->exists(thread->id), true);
        assert_eq("sch.procs.threads.new.id", process->threads->peek()->id, thread->id);

        // Schedule the thread.
        thread_execution_queue->push(thread);

        assert_eq("sch.procs.threads.exec.new.id", thread_execution_queue->peek()->id, thread->id);

        return thread;
    }

    void kill_thread(Thread* thread, uint32_t code, bool remove_from_process) {
        // Iterate through thread process list and remove.
        auto thread_process_iterator = thread->process->threads->create_iterator();

        if (remove_from_process) {
            while (thread_process_iterator.hasNext()) {
                Thread* p_thread = thread_process_iterator.next();

                if (p_thread == thread) thread_process_iterator.remove();
            }
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

        assert_eq("sch.procs.threads.delete.heap", heap::allocated(thread), false);
    }

    void kill_process(Process* process, uint32_t code) {
        assert_eq("sch.procs.list.exists.1", process_list->exists(process->id), true);

        // Remove the process from map.
        process_list->remove(process->id);
        process_name_list->remove(process->name);

        assert_eq("sch.procs.list.exists.2", process_list->exists(process->id), false);

        // Iterate through the processes threads and terminate.
        auto process_threads_iterator = process->threads->create_iterator();

        while (process_threads_iterator.hasNext()) {
            Thread* thread = process_threads_iterator.next();
            
            kill_thread(thread, 1, false);
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

        assert_eq("sch.procs.delete.heap", heap::allocated(process), false);
    }

    void manual_context_switch_return() {
        // Schedule the thread.
        thread_execution_queue->push(current_thread);

        // Clear current thread.
        current_thread = nullptr;

        // Restore temporary registers.
        temporary_registers = &placeholder_registers;
    }
}