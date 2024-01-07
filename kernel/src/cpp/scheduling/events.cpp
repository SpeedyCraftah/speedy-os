#include "events.h"

#include "scheduler.h"
#include "stdint.h"

#include "structures/process.h"
#include "structures/thread.h"
#include "structures/events.h"

namespace scheduler {
  namespace events {
    void emit_event(Process* sender_process, uint32_t id, uint32_t data) {
        if (sender_process->hooked_threads->get_size() == 0) return;

        // Create iterator.
        auto thread_listener_iterator = sender_process->hooked_threads->create_iterator();

        // Iterate over hooked threads.
        while (thread_listener_iterator.hasNext()) {
            ThreadEventListener event_listener = thread_listener_iterator.next();

            // If thread is not listening to event id, continue.
            if ((event_listener.enabled_event_ids & id) == 0) {
                continue;
            }

            // If thread no longer exists, remove.
            if (!scheduler::thread_list->exists(event_listener.thread_id)) {
                thread_listener_iterator.remove();
                continue;
            }

            // Improve efficiency.
            Thread* thread = scheduler::thread_list->fetch(event_listener.thread_id);

            // If the process cannot accept events, continue.
            if (thread->execution_policy == ThreadExecutionPolicy::BUSY) {
                continue;
            }

            // Create the event.
            ThreadEvent event;
            event.event_id = id;
            event.event_data = data;
            event.handler = event_listener.handler;
            event.thread = thread;

            // Add the event to the queue.
            scheduler::thread_event_queue->push(event);
        }
    }
  }
}