#pragma once

#include "stdint.h"

enum TaskPriority : uint8_t {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    REALTIME = 3
};

enum SuspensionType {
    FULL = 0, // program completely suspended (including events).
    EVENTS_ONLY = 1 // program suspended, only events will be given CPU time.
};

enum TaskStatus : uint8_t {
    RUNNING = 0, // running - will run on its own accord and receive events.
    SUSPENDED = 1, // suspended - no cpu time given (even if events occur).
    RUNNING_WAITING_FOR_DATA = 2, // waiting - will be given cpu time only when an event occurs.
    RUNNING_BUSY = 3  // running but busy - all events will be supressed until status 0 or 2.
};
