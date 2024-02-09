#pragma once

#include "stdint.h"

#include "../../../../../shared/linked_array.h"
#include "../../paging/paging.h"
#include "registers.h"
#include "datasink.h"

enum ProcessTermCode : uint32_t {
    PAGE_ACCESS_VIOLATION = 100,
    GENERAL_PROTECTION_FAULT
};

struct ProcessFlags {
    uint8_t kernel_process : 1; // Program has direct access to the kernel in its address space.
    uint8_t iopl_0 : 1; // Program runs with highest IO privileges (IOPL=0).
    uint8_t virtual_process : 1; // Program only exists for the purpose of events and other symbolic purposes, cannot execute.
    uint8_t interface_provider : 1; // Program is an interface provider and allows interaction with kernel.
};

struct ThreadKernelArea {
    bool allocated;
    Registers registers;
};

struct Thread;
struct ThreadEventListener;

// TODO - make certain features such as hooked_events or sinks optional so not to allocate for no reason.
struct Process {
    uint32_t id;

    ProcessFlags flags;

    struct state {
        bool suspended : 1;
    } state;

    struct paging {
        ThreadKernelArea* kernel_thread_page;
        PageDirectory* directories;
        uint32_t pixel_mapping_address;
    } paging;

    structures::linked_array<Thread*>* threads = nullptr;
    structures::linked_array<ThreadEventListener>* hooked_threads = nullptr;
    structures::map<SteadyDataSink*>* steady_sinks = nullptr;

    char* name;
    float total_cpu_time;
};