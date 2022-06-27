#include "semaphore.h"
#include "sys.h"

ThreadSemaphore::ThreadSemaphore(unsigned int initial_value) {
    mutex = ThreadMutex();
    counter = initial_value;
}

void ThreadSemaphore::wait() {
    while (true) {
        mutex.lock();

        if (counter > 0) {
            --counter;
            mutex.unlock();
            return;
        }

        mutex.unlock();

        // Preempt as the OS is single core anyways.
        speedyos::preempt_thread();
    }
}

void ThreadSemaphore::signal() {
    mutex.lock();

    ++counter;

    mutex.unlock();
}