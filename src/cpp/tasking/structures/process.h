#pragma once

#include "stdint.h"
#include "registers.h"
#include "state.h"
#include "../../structures/string.h"

enum ProcessFlag : uint32_t {
    SYSTEM_PROCESS = 1
};

struct Process {
    structures::string name;

    uint32_t id;
    ProcessFlag flags;

    TaskStatus status;
    uint32_t suspended_until;
    
    TaskPriority priority;
    Registers registers;
    
    uint32_t total_cpu_time;

    void* stack_base;
};
