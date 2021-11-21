#pragma once

#include "stdint.h"
#include "registers.h"
#include "state.h"
#include "../../structures/string.h"
#include "events.h"

enum ProcessFlag : uint32_t {
    SYSTEM_PROCESS = 1,

    // Drivers will not be allocated a spot in heap & will not run.
    // Will only be allocated a virtual process in order for registering
    // & dispatching to take place.
    SYSTEM_DRIVER = 2,

    INTERFACE_PROVIDER = 4
};

struct Process {
    structures::string name;

    uint32_t id;
    uint32_t flags;

    TaskStatus current_status;
    TaskStatus main_status;
    SuspensionType suspended_type;
    uint32_t suspended_until;
    
    TaskPriority priority;
    Registers registers;

    TaskEventReceiver event_receiver;
    TaskEventEmitter event_emitter;
    
    uint32_t total_cpu_time;

    void* stack_base;
};
