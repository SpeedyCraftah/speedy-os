#pragma once

#include "stdint.h"
#include "../../structures/flex_array.h"
#include "./registers.h"

struct TaskEvent {
    uint32_t event_id;
    uint32_t event_data;
    void* handler_ptr;
    bool overtime;
};

struct TaskEventReceiver {
    bool supported;

    Registers registers;
    void* stack_base;

    // Queued events.
    structures::flexible_array<TaskEvent>* queue;
};

struct TaskEventSubscription {
    // Bit field of enabled event IDs.
    uint32_t enabled_event_ids;
    void* handler_ptr;
    uint32_t process_id;
};

struct TaskEventEmitter {
    bool supported;

    structures::flexible_array<TaskEventSubscription>* subscribed;
};