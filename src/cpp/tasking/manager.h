#pragma once

#include "../../software/process_t.h"
#include "../structures/map.h"
#include "structures/process.h"

namespace multitasking {
    static void start_service();

    class process_manager {
        public:
            // ID of the currently running process. -1 if kernel.
            static int current_process_id;
            static unsigned int next_process_id;
            
            static void start_process(char* name, Software::ProcessRuntime* process, TaskPriority priority = TaskPriority::NORMAL, ProcessFlag flags = (ProcessFlag)0);

            static structures::map<int, Process> processes;

    };

    namespace asm_handlers {
        extern "C" void on_process_yield();
    }
}