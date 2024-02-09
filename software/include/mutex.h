#pragma once

#include "../../shared/linked_array.h"

class ThreadMutex {
    public:
        ThreadMutex();
        ~ThreadMutex();

        // Locks the mutex resource.
        // If the resource is already in use, the thread will be parked and wait for the resource to be free.
        bool lock();

        // Unlocks the mutex resource.
        void unlock();

        // Returns whether the resource is currently locked.
        bool locked();

    private:
        structures::linked_array<uint32_t>* thread_queue;
        uint32_t locked_by_thread = 0;
};