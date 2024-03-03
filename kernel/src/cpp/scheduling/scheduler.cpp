#include "scheduler.h"

#include "datasink.h"
#include "events.h"
#include "stdint.h"

#include "../../../../shared/flex_array.h"
#include "../../../../shared/map.h"

#include "structures/events.h"
#include "structures/process.h"
#include "structures/thread.h"
#include "events.h"
#include "../../../../shared/memory.h"
#include "../heap/physical.h"
#include "../paging/virtual.h"
#include "../paging/paging.h"
#include "structures/datasink.h"

// remove after
#include "../../../../shared/conversions.h"
//#include "../software/include/sys.h"
#include "../chips/pit.h"
#include "../misc/assert.h"
#include "../io/video.h"
#include "syscalls.h"

extern "C" volatile int bpwatch;

// Temporary structure until ESP switch on interrupt is implemented.
extern "C" InterruptFrame temporary_interrupt_frame = InterruptFrame();

// Temporary registers switched to pointer for performance.
extern "C" Registers placeholder_registers = Registers();
extern "C" Registers* temporary_registers = nullptr;
extern "C" void* virtual_temporary_registers = nullptr;
extern "C" void* temporary_eip = nullptr;

// Import the assembly functions.
extern "C" void scheduler_sleep();
extern "C" void scheduler_execute();
extern "C" void save_fpu_boot_state();
extern "C" void debug();

// If defined the scheduler debug runtime tools will be compiled and enabled.
// Also enables asserts in areas where there is performance impact of doing so.
#define SCHEDULER_DEBUG

namespace scheduler {
    uint32_t elapsed_ms = 0;
    uint32_t time_slice_ms = 5;
    uint32_t tsc_tick_rate = 0;
    uint32_t scheduler_idle_time_ms = 0;
    uint32_t last_scheduler_idle_time = 0;
    uint32_t scheduler_running_time_ms = 0;
    uint32_t last_scheduler_running_time_ms = 0;
    bool debug_mode = false;
    char fpu_boot_state[108];
    Process* kernel_process;

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

    Process* scheduler_event_process = nullptr;
    Process* interface_provider_events_process = nullptr;
    uint32_t interface_provider_output_sink_id = 0;

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
        flags.kernel_process = true;
        flags.virtual_process = true;

        scheduler_event_process = create_process("Scheduler Events", 0, flags);

        assert_eq("sch.procs.1.name.exists", process_name_list->exists("Scheduler Events"), true);
        assert_eq("sch.procs.1.id", process_name_list->fetch("Scheduler Events")->id, scheduler_event_process->id);
        assert_eq("sch.procs.1.id.exists", process_list->exists(scheduler_event_process->id), true);

        // Create kernel process for usage by virtual functions.
        kernel_process = new Process;
        kernel_process->paging.directories = paging::kernel_page_directory;

        PageDirectory placeholder_dir = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // Set all page directories to 0 by default.
        for (uint32_t i = 0; i < 1024; i++) {
            paging::kernel_page_directory[i] = placeholder_dir;        
        }

        // Map all kernel pages 1:1.
        // TODO - only map necessary kernel pages, dynamically allocate later.
        for (uint32_t i = 0; i < 1024 * 1024; i++) {
            PageEntry* page = virtual_allocator::fetch_page_index(scheduler::kernel_process, i, true);
            page->Address = i;
            page->ReadWrite = true;
            page->Present = true;
        }

        // Set first 100M to global for performance.
        for (uint32_t j = 0; j < 25 * 1024; j++) {
            PageEntry* entry = virtual_allocator::fetch_page_index(scheduler::kernel_process, j, true);
            entry->Global = true;
        }

        // Initialise datasinks.
        datasink::initialise();
    }

    // TODO - make paging inline asm functions
    extern "C" void handle_context_switch() {
        // If a thread was previously running.
        if (current_thread != nullptr) {
            // Switch to kernel paging.
            paging::switch_directory(paging::kernel_page_directory);

            uint32_t cpu_time_used =
                timer_preempt ? time_slice_ms : (time_slice_ms - (uint32_t)chips::pit::fetch_channel_0_remaining_countdown());
            
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
                assert_eq("sch.events.thread.heap", kallocated(event.thread), true);
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
            memcpy(&event.thread->backup_registers, event.thread->registers, sizeof(Registers));

            // Set the EIP to the event handler.
            event.thread->registers->eip = reinterpret_cast<uint32_t>(event.handler);

            // Mark the thread as busy to queue future events.
            event.thread->state.processing_event = true;

            // Create a stack frame.

            // Make space for the data parameters so we don't overwrite the program stack.
            // Use normal execution programs ESP as base.
            event.thread->registers->esp = event.thread->backup_registers.esp - sizeof(uint32_t) * (2 + 1);

            uint32_t virtual_difference = (reinterpret_cast<uint32_t>(event.thread->virtual_stack) + 8191) - event.thread->registers->esp;
            uint32_t* physical_stack = reinterpret_cast<uint32_t*>((reinterpret_cast<uint32_t>(event.thread->physical_stack) + 8191) - virtual_difference);

            // Push the event data.
            // Return address?
            *(physical_stack + 0) = 0xd3adb33f;
            *(physical_stack + 1) = event.event_id;
            *(physical_stack + 2) = event.event_data;

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
            // WARNING - this is a virtual address to the registers.
            virtual_temporary_registers = thread->virtual_registers;
            temporary_registers = (Registers*)thread->registers;
            temporary_eip = reinterpret_cast<void*>(thread->registers->eip);

            // Switch to process paging.
            paging::switch_directory(thread->process->paging.directories);

            // Switch to assembly side of scheduler to begin execution.
            return scheduler_execute();
        }

        // If there is nothing available to execute, add sleep time and sleep.
        
        scheduler_idle_time_ms += (uint32_t)chips::pit::fetch_channel_0_remaining_countdown();

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

    Process* create_process(char* name, void (*entry)(), ProcessFlags flags, bool schedule_immediately) {
        // Create the process.
        Process* process = new Process;
        process->id = next_process_id++;
        process->name = structures::string(name).char_copy().norm();
        process->flags = flags;
        process->hooked_threads = new structures::linked_array<ThreadEventListener>(6);
        process->steady_sinks = new structures::map<DataSinkPermissions>(4);

        assert_eq("sch.procs.name.heap", kallocated(process->name), true);
        assert_eq("sch.procs.hookedthreads.heap", kallocated(process->hooked_threads), true);

        // Add the process to the map.
        process_list->set(process->id, process);
        process_name_list->set(process->name, process);
        
        // If the process is not a virtual process.
        // Create main thread.
        // Virtual processes have no running thread.
        if (!flags.virtual_process) {
            // Create non-virtual structures.
            process->threads = new structures::linked_array<Thread*>(8);

            assert_eq("sch.procs.threads.heap", kallocated(process->threads), true);

            Thread* thread = new Thread;
            thread->id = next_thread_id++;
            thread->process = process;
            thread->flags.main_thread = true;

            // Allocate physical page to store thread-kernel shared data.
            // At the moment only used to store registers so that registers can be saved/loaded without any copying overhead due to paging.
            void* shared_page = physical_allocator::alloc_physical_page(1, true);
            process->paging.kernel_thread_page = (ThreadKernelArea*)shared_page;

            // Allocate first block for main thread.
            thread->kernel_thread_data = (ThreadKernelArea*)shared_page;
            thread->kernel_thread_data->allocated = true;
            thread->registers = &thread->kernel_thread_data->registers;
            thread->virtual_registers = (void*)(104857600 + 4096 + 4);

            thread->registers->eip = reinterpret_cast<uint32_t>(entry);

            process->paging.pixel_mapping_address = 0;

            // Load the setup FPU state.
            memcpy(thread->registers->fpu_state, fpu_boot_state, sizeof(fpu_boot_state));

            // Allocate a page directory.
            process->paging.directories = (PageDirectory*)physical_allocator::alloc_physical_page(1, true);

            // Set up the page directories.
            for (uint32_t i = 0; i < 1024; i++) {
                PageDirectory& directory = process->paging.directories[i];

                // These properties are handled on a page-by-page level.
                directory.UserSupervisor = true;
                directory.Present = false;
                directory.ReadWrite = true;
            }

            // Map first 100M of virtual memory to physical.
            // Allocate directories needed for 100M of memory.
            for (uint32_t i = 0; i < 104857600 / 4096 / 1024; i++) {
                PageEntry* tables = (PageEntry*)physical_allocator::alloc_physical_page();

                PageDirectory& directory = process->paging.directories[i];
                directory.Present = true;
                directory.Address = paging::address_to_pi(tables);
            
                // Map the individual pages.
                for (uint32_t j = 0; j < 1024; j++) {
                    PageEntry& entry = tables[j];
                    entry.Address = (i * 1024) + j;
                    entry.Present = true;
                    entry.ReadWrite = true;
                    entry.Global = false;
                    entry.UserSupervisor = process->flags.kernel_process;
                }
            }

            // Map the shared page to be positioned right after the 100M 1-1 mapped pages.
            // Add one additional page before to act as a guard page.
            // TODO - check if page is set to ring 0 + check for undefined behaviour on deallocate.
            PageEntry* shared_virtual_page = virtual_allocator::fetch_page_index(process, paging::address_to_pi((void*)(104857600 + 4096)), true);
            shared_virtual_page->Present = true;
            shared_virtual_page->Address = paging::address_to_pi(shared_page);
            shared_virtual_page->ReadWrite = true;

            // TODO - temporarily map graphics memory to space

            // Allocate 2 pages for the stack.
            void* physical_stack_offset = physical_allocator::alloc_physical_page(2, true);
            uint32_t virtual_stack_offset = virtual_allocator::find_free_virtual_pages(process, 2);

            // Map to a virtual address.
            // TODO - map to some pre-set area.
            for (int i = 0; i < 2; i++) {
                PageEntry* page = virtual_allocator::fetch_page_index(process, virtual_stack_offset + i);
                page->Present = true;
                page->ReadWrite = true;
                page->Address = paging::address_to_pi(physical_stack_offset) + i;
                page->UserSupervisor = true;
            }

            // Set the stack addresses on thread.
            thread->physical_stack = physical_stack_offset;
            thread->virtual_stack = paging::pi_to_address(virtual_stack_offset);

            // Point to top of stack.
            thread->registers->esp = reinterpret_cast<uint32_t>(thread->virtual_stack) + 8192 - sizeof(uint32_t);

            // Create a default EFLAGS.
            // https://en.wikibooks.org/wiki/X86_Assembly/X86_Architecture
            thread->registers->eflags = process->flags.iopl_0 ? 0b00000000001000000000001000000010 : 0b00000000001000000011001000000010;

            // Add the thread to the map.
            thread_list->set(thread->id, thread);
        
            // Add thread to process.
            process->threads->push(thread);

            // Schedule the thread.
            if (schedule_immediately) thread_execution_queue->push(thread);
        }

        // Emit process create event.
        if (scheduler_event_process != nullptr) scheduler::events::emit_event(scheduler_event_process, 1, process->id);

        // Return the process.
        return process;
    }

    Thread* create_thread(Process* process, void (*entry)(), void* capture) {
        Thread* thread = new Thread;
        thread->id = next_thread_id++;
        thread->process = process;
        thread->kernel_thread_data = nullptr;

        // Find first free shared block.
        // TODO - add support for more than 4096/sizeof(ThreadKernelArea) blocks.
        for (int i = 0; i < 4096 / sizeof(ThreadKernelArea); i++) {
            ThreadKernelArea& block = process->paging.kernel_thread_page[i];
            if (block.allocated) continue;
            
            // Allocate the block.
            block.allocated = true;
            thread->kernel_thread_data = &process->paging.kernel_thread_page[i];
            thread->registers = &thread->kernel_thread_data->registers;

            // WARNING - this is a virtual address.
            thread->virtual_registers = (void*)(104857600 + 4096 + 4 + (i * sizeof(ThreadKernelArea)));
        
            // Clear block for use.
            // TODO - strictly not needed, check if should be removed for performance.
            memset(&thread->kernel_thread_data->registers, 0, sizeof(Registers));

            break;
        }

        // Check if block could not be allocated.
        if (thread->kernel_thread_data == nullptr) [[unlikely]] {
            kernel::panic("Shared data block for thread could not be allocated.");
            __builtin_unreachable();
        }
        
        thread->registers->eip = reinterpret_cast<uint32_t>(entry);

        assert_eq("sch.procs.threads.new.heap", kallocated(thread), true);
        assert_eq("sch.procs.threads.new.proc.heap", kallocated(process), true);

        // Load the setup FPU state.
        memcpy(thread->registers->fpu_state, fpu_boot_state, sizeof(fpu_boot_state));
        
        // Allocate 2 pages for the stack.
        void* physical_stack_offset = physical_allocator::alloc_physical_page(2, true);
        uint32_t virtual_stack_offset = virtual_allocator::find_free_virtual_pages(process, 2);

        // Map to a virtual address.
        // TODO - map to some pre-set area.
        for (int i = 0; i < 2; i++) {
            PageEntry* page = virtual_allocator::fetch_page_index(process, virtual_stack_offset + i);
            page->Present = true;
            page->ReadWrite = true;
            page->Address = paging::address_to_pi(physical_stack_offset) + i;
            page->UserSupervisor = true;
        }

        // Set the stack addresses on thread.
        thread->physical_stack = physical_stack_offset;
        thread->virtual_stack = paging::pi_to_address(virtual_stack_offset);

        // Point to top of stack.
        thread->registers->esp = reinterpret_cast<uint32_t>(thread->virtual_stack) + 8192 - sizeof(uint32_t);

        // Place the capture on the stack.
        thread->registers->esp -= sizeof(uint32_t) * 2;
        //uint32_t* stack = reinterpret_cast<uint32_t*>(thread->registers.esp);

        // Create a default EFLAGS.
        // https://en.wikibooks.org/wiki/X86_Assembly/X86_Architecture
        thread->registers->eflags = 0b00000000001000000011001000000010;

        // TODO - check this probably doesnt work.
        uint32_t* stack = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(physical_stack_offset) + 8192 - sizeof(uint32_t));
        *stack = reinterpret_cast<uint32_t>(capture);

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
        // Remove from global thread map.
        thread_list->remove(thread->id);

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

        // Mark shared kernel block as deallocated.
        thread->kernel_thread_data->allocated = false;

        // Deallocate and remove.
        virtual_allocator::reset_virtual_pages(thread->process, paging::address_to_pi(thread->virtual_stack), 2, true);
        delete thread;

        assert_eq("sch.procs.threads.delete.heap", kallocated(thread), false);
    }

    void kill_process(Process* process, uint32_t code) {
        // Scheduler clean-up if process was running.
        if (scheduler::current_thread && scheduler::current_thread->process == process) {
            scheduler::current_thread = nullptr;
            scheduler::temporary_registers = &scheduler::placeholder_registers;
        }

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

        // Iterate through the data sinks and delete if process owns them.
        auto steady_sinks_iterator = process->steady_sinks->create_iterator();

        while (steady_sinks_iterator.hasNext()) {
            auto entry = steady_sinks_iterator.next();
            DataSinkPermissions sink_permissions = entry.value();
            if (!sink_permissions.owner) continue;

            // Sinks should only be deleted by the process that owns them.
            assert_eq("sch.procs.sinks.clean.exists", datasink::active_sinks.exists(entry.key()), true);
            
            // Delete the datasink.
            SteadyDataSink* sink = datasink::active_sinks.fetch(entry.key());
            scheduler::datasink::active_sinks.remove(entry.key());
            delete sink;
        }

        // Emit process end event.
        scheduler::events::emit_event(scheduler_event_process, 2, process->id);

        // Free all page directories and physical pages used by process.
        for (uint32_t i = 0; i < 1024; i++) {
            PageDirectory& directory = process->paging.directories[i];
            if (!directory.Present) continue;

            PageEntry* entries = (PageEntry*)paging::pi_to_address(directory.Address);
            for (uint32_t j = 0; j < 1024; j++) {
                PageEntry& entry = entries[j];
                if (!entry.Present || (((entry.KernelFlags & PageFlags::PROCESS_OWNED) == 0) && ((entry.KernelFlags & PageFlags::RELEASE_PHYSICAL_ON_EXIT) == 0))) continue;

                // Free the physical page.
                physical_allocator::free_physical_page(paging::pi_to_address(entry.Address));
            }

            // Free the physical page holding the entries for the directory.
            physical_allocator::free_physical_page(entries);
        }

        // Free root page directory.
        physical_allocator::free_physical_page(process->paging.directories);

        // Free process objects.
        if (process->threads != nullptr) delete process->threads;
        delete process->hooked_threads;
        delete process->steady_sinks;
        physical_allocator::free_physical_page(process->paging.kernel_thread_page);
        kfree(process->name);

        // Remove the process itself.
        delete process;

        assert_eq("sch.procs.delete.heap", kallocated(process), false);
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