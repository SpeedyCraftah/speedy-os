#include "mutex.h"

#include "stdint.h"
#include "sys.h"
#include "alloc.h"

ThreadMutex::ThreadMutex() {
    // Create the queue.
    thread_queue = new structures::linked_array<uint32_t>(5);
}

ThreadMutex::~ThreadMutex() {
    delete thread_queue;
}

bool ThreadMutex::lock() {
    // If the resource is locked.
    if (locked_by_thread != 0) {
        // Push to queue. (to be changed to system call).
        // TODO: change to cached thread ID.
        thread_queue->push(speedyos::fetch_thread_id());

        // Park the thread.
        // Keep the thread parked until resource is free.
        while (locked_by_thread != 0) {
            speedyos::park_thread();
        }
    }

    // Lock the resource.
    // TODO: here too.
    locked_by_thread = speedyos::fetch_thread_id();

    return true;
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