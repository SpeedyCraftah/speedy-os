#include "mutex.h"

#include "stdint.h"
#include "sys.h"
#include "../../scheduling/scheduler.h"

ThreadMutex::ThreadMutex() {
    // Create the queue.
    thread_queue = new structures::linked_array<uint32_t>(5);
}

ThreadMutex::~ThreadMutex() {
    delete thread_queue;
}

void ThreadMutex::lock() {
    // If the resource is locked.
    if (locked_by_thread != 0) {
        // Push to queue. (to be changed to system call).
        thread_queue->push(scheduler::current_thread->id);

        // Park the thread.
        // Keep the thread parked until resource is free.
        while (locked_by_thread != 0) {
            speedyos::park_thread();
        }
    }

    // Lock the resource.
    locked_by_thread = scheduler::current_thread->id;
}

void ThreadMutex::unlock() {
    // Unlock the resource.
    locked_by_thread = 0;

    // If there are threads waiting for the resource.
    if (thread_queue->get_size() != 0) {
        // Remove latest from queue and unpark.
        speedyos::awake_thread(thread_queue->shift());
    }
}

bool ThreadMutex::locked() {
    return locked_by_thread != 0;
}