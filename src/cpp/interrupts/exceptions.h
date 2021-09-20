#pragma once

#include <stdint.h>
#include "../structures/events.h"

namespace interrupts {
    class exceptions {
        public:
            static structures::event_handler<uint8_t, uint8_t> events;
            static void load_all();
    };
}