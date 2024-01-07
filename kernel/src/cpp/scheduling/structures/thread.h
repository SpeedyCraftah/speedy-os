#pragma once

#include "stdint.h"

#include "process.h"

enum ThreadExecutionPolicy {
    NORMAL,
    EVENT_ONLY,
    BUSY
};

struct Thread {
    uint32_t id;
    Registers* registers;
    void* virtual_registers;
    Registers backup_registers;

    struct flags {
        bool main_thread : 1;
    } flags;

    ThreadExecutionPolicy execution_policy;

    struct state {
        bool suspended : 1;
        bool parked : 1;
        bool processing_event : 1;
    } state;

    Process* process;

    struct suspension_details {
        uint32_t resume_time;
    } suspension_details;
    
    void* virtual_stack;
    void* physical_stack;
    ThreadKernelArea* kernel_thread_data;
};