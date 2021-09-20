// Controller written in C++ since assembly would be too much work.

// PIC - Responsible for IRQ interrupts (keyboard/timer etc).
// Dedicated chip which needs communicating with.

#pragma once

#include <stdint.h>

namespace controllers {
    namespace pic {
        void remap(uint8_t master_offset, uint8_t slave_offset);
        
        void mask_line(uint8_t line);
        void unmask_line(uint8_t line);
    }
}