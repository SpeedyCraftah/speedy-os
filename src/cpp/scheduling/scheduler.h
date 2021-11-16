// Handles the clock, ticking and program switching as well as task management.

#pragma once

#include <stdint.h>

#include "./structures/process.h"

#include "../structures/flex_array.h"
#include "../structures/map.h"

namespace scheduler {
        extern "C" uint32_t current_process;
        extern "C" uint32_t elapsed_ms;
        extern "C" uint32_t kernel_stack_base;
        extern "C" uint32_t kernel_stack_pointer;
        extern "C" bool event_running;

        void initialise();

        structures::map<Process*>* get_process_list();
        structures::flexible_array<uint32_t>* get_process_queue();
        
        structures::map<Process*>* get_process_list_string();

        uint32_t start_process(structures::string name, void(*entry)() = 0, TaskStatus status = TaskStatus::RUNNING, uint32_t flags = 0, bool event_receiver_support = false, bool event_emitter_support = false);
        void end_process(uint32_t process_id, uint32_t code);
}
