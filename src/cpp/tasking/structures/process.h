#pragma once

#include "stdint.h"
#include "registers.h"
#include "state.h"
#include "../../../software/process_t.h"

enum ProcessFlag : uint32_t {
    SYSTEM_PROCESS = 1
};

struct Process {
    char* name;
    uint32_t id;
    ProcessFlag flags;

    TaskPriority priority;
    Registers registers;

    Software::ProcessRuntime* process;
};