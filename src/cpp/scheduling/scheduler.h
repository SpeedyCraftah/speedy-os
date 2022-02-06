#pragma once

#include "stdint.h"

#include "../structures/map.h"
#include "../structures/linked_array.h"

#include "structures/thread.h"
#include "structures/process.h"
#include "structures/events.h"

namespace scheduler {
    extern uint32_t elapsed_ms;

    extern "C" Thread* current_thread;

    extern structures::map<Thread*>* thread_list;
    extern structures::map<Process*>* process_list;

    extern structures::map<Process*>* process_name_list;

    extern structures::linked_array<ThreadEvent>* thread_event_queue;
    extern structures::linked_array<Thread*>* thread_execution_queue;

    void initialise();
    extern "C" void handle_context_switch();

    Process* create_process(char* name, void (*entry)(), ProcessFlags flags = ProcessFlags());
    Thread* create_thread(Process* process, void (*entry)());

    void kill_process(Process* process, uint32_t code = 1);
    void kill_thread(Thread* thread, uint32_t code = 1);

    // Utility function.
    void manual_context_switch_return();
}