// PIT - A programmable timer chip used to set interrupts every x depending on frequency.
// Used for multi-tasking and time-keeping.
#define DIVISOR 1193180

#pragma once

#include <stdint.h>
#include "../abstractions/io_port.h"

namespace chips {
    namespace pit {
        void set_channel_0_frequency(uint16_t frequency_hz);

        inline float fetch_channel_0_remaining_countdown() {
            uint16_t remaining_time = io_port::bit_8::in(0x40);
            remaining_time += io_port::bit_8::in(0x40) << 8;

            return 1000 / ((float)DIVISOR / remaining_time);
        }
    }
}