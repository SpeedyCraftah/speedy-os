#pragma once

#include "stdint.h"

#include "thread.h"

struct ThreadEvent {
    Thread* thread;
    
    uint32_t event_id;
    uint32_t event_data;
    
    void* handler;
};

struct ThreadEventListener {
    uint32_t thread_id;
    uint32_t enabled_event_ids;

    void* handler;
};