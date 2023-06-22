#pragma once

#include "stdint.h"

#include "../structures/map.h"
#include "../structures/linked_array.h"

#include "structures/thread.h"
#include "structures/process.h"
#include "structures/events.h"

namespace scheduler {
    extern uint32_t elapsed_ms;
    extern uint32_t time_slice_ms;
    extern float scheduler_idle_time_ms;
    extern float last_scheduler_idle_time;
    extern float scheduler_running_time_ms;
    extern float last_scheduler_running_time_ms;
    extern bool debug_mode;
    extern uint32_t tsc_tick_rate;
    extern "C" char fpu_boot_state[108];
    
    extern "C" bool timer_preempt;
    extern "C" Thread* current_thread;
    extern "C" void* virtual_temporary_registers;
    extern "C" Registers* temporary_registers;
    extern "C" Registers placeholder_registers;

    extern structures::map<Thread*>* thread_list;
    extern structures::map<Process*>* process_list;

    extern structures::map<Process*>* process_name_list;

    extern structures::linked_array<ThreadEvent>* thread_event_queue;
    extern structures::linked_array<Thread*>* thread_execution_queue;

    void initialise();
    extern "C" void handle_context_switch();

    Process* create_process(char* name, void (*entry)(), ProcessFlags flags = ProcessFlags());
    Thread* create_thread(Process* process, void (*entry)(), void* capture = 0);

    void kill_process(Process* process, uint32_t code = 1);
    void kill_thread(Thread* thread, uint32_t code = 1, bool remove_from_process = true);

    // Utility function.
    void manual_context_switch_return();
}