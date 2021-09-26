// PIT - A programmable timer chip used to set interrupts every x depending on frequency.
// Used for multi-tasking and time-keeping.

#pragma once

#include <stdint.h>

namespace chips {
    namespace pit {
        void set_channel_0_frequency(uint16_t frequency_hz);
    }
}