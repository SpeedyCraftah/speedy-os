#pragma once

#include "stdint.h"

#include "../../structures/linked_array.h"

struct ProcessFlags {
    uint8_t system_process : 1;
    uint8_t virtual_process : 1;
    uint8_t interface_provider : 1;
};

struct Thread;
struct ThreadEventListener;

struct Process {
    uint32_t id;

    ProcessFlags flags;

    struct state {
        bool suspended : 1;
    } state;

    structures::linked_array<Thread*>* threads;
    structures::linked_array<ThreadEventListener>* hooked_threads;

    char* name;
};