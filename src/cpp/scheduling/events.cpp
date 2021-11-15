#include "events.h"
#include "scheduler.h"

namespace scheduler {
    namespace events {
        bool add_event_listener(
            uint32_t receiver_process_id, uint32_t emitter_process_id, uint32_t enabled_events_bitfield, uint32_t handler_ptr
        ) {
            Process* receiver = scheduler::get_process_list()->fetch(receiver_process_id);
            Process* emitter = scheduler::get_process_list()->fetch(emitter_process_id);

            // If emitter does not support event emitting.
            if (!emitter->event_emitter.supported) return false;

            // If receiver does not support event emitting.
            if (!receiver->event_receiver.supported) return false;

            TaskEventSubscription subscription;
            subscription.enabled_event_ids = enabled_events_bitfield;
            subscription.process_id = receiver_process_id;
            subscription.handler_ptr = (void*)handler_ptr;

            emitter->event_emitter.subscribed->push(subscription);

            return true;
        }

        void remove_event_listener(
            uint32_t receiver_process_id, uint32_t emitter_process_id
        ) {
            Process* emitter = scheduler::get_process_list()->fetch(emitter_process_id);

            // If emitter does not support emitting events.
            if (!emitter->event_emitter.supported) return;

            for (uint32_t i = 0; i < emitter->event_emitter.subscribed->get_size(); i++) {
                TaskEventSubscription subscription = emitter->event_emitter.subscribed->get_at(i);
                
                if (subscription.process_id != receiver_process_id) continue;
                else {
                    // Remove listener.
                    emitter->event_emitter.subscribed->remove_at(i);

                    // Defragment array.
                    emitter->event_emitter.subscribed->defragment();

                    // We're done here!
                    break;
                }
            }
        }

        void emit_event(
            uint32_t emitter_process_id,
            uint32_t event_id,
            uint32_t event_data
        ) {

            Process* emitter = scheduler::get_process_list()->fetch(emitter_process_id);

            if (!emitter->event_emitter.supported) return;

            for (uint32_t i = 0; i < emitter->event_emitter.subscribed->get_size(); i++) {
                TaskEventSubscription subscription = emitter->event_emitter.subscribed->get_at(i);
                
                // If event ID is not whitelisted, skip.
                if ((subscription.enabled_event_ids & event_id) == 0) continue;

                // If process no longer exists.
                if (!scheduler::get_process_list()->exists(subscription.process_id)) {
                    // Remove from subscription list and continue.
                    emitter->event_emitter.subscribed->remove_at(i);
                    emitter->event_emitter.subscribed->defragment();

                    continue;
                }

                Process* receiver = scheduler::get_process_list()->fetch(subscription.process_id);

                // If the receiver has events suppressed, do not queue.
                if (receiver->main_status == TaskStatus::RUNNING_BUSY) continue;

                // Schedule and push event.

                TaskEvent event;
                event.event_id = event_id;
                event.event_data = event_data;
                event.handler_ptr = subscription.handler_ptr;

                receiver->event_receiver.queue->push(event);
            }
        }
    }
}