#pragma once

#include "scheduler.h"

namespace scheduler {
    namespace events {
        bool add_event_listener(
            uint32_t receiver_process_id, uint32_t emitter_process_id, uint32_t enabled_events_bitfield, uint32_t handler_ptr
        );

        void remove_event_listener(
            uint32_t receiver_process_id, uint32_t emitter_process_id
        );

        void emit_event(
            uint32_t emitter_process_id,
            uint32_t event_id,
            uint32_t event_data
        );
    }
}