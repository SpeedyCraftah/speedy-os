#pragma once

#include "stdint.h"

#include "../../structures/linked_array.h"
#include "../../paging/paging.h"
#include "registers.h"

struct ProcessFlags {
    uint8_t system_process : 1;
    uint8_t virtual_process : 1;
    uint8_t interface_provider : 1;
};

struct ThreadKernelArea {
    bool allocated;
    Registers registers;
};

struct Thread;
struct ThreadEventListener;

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

    structures::linked_array<Thread*>* threads;
    structures::linked_array<ThreadEventListener>* hooked_threads;

    char* name;
    float total_cpu_time;
};