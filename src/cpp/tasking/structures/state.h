#pragma once

#include "stdint.h"

enum TaskPriority : uint8_t {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    REALTIME = 3
};

// actively running = Program processes on its own accord.

enum TaskStatus : uint8_t {
    RUNNING = 0, // running - will run on its own accord (while loop) and receive events.
    SUSPENDED = 1, // suspended - no cpu time given (even if events occur).
    WAITING_FOR_DATA = 2, // waiting - will be given cpu time when an event occurs.
    RUNNING_BUSY = 3  // running but busy - all events will be supressed until status 0 or 2.
};