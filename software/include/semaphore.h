// I don't exactly understand them but I'll implement them because I need them and they're simple enough.

#pragma once

#include "mutex.h"

class ThreadSemaphore {
    public:
        ThreadSemaphore(unsigned int initial_value = 0);
        ~ThreadSemaphore();

        void wait();
        void signal();

    private:
        ThreadMutex mutex;
        uint32_t counter;
};