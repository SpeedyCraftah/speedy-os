#pragma once

#include <stdint.h>
#include "../structures/events.h"

namespace interrupts {
    class irq {
        public:
            static uint32_t IRQ0_HANDLER;
            static uint32_t IRQ1_HANDLER;
        
            static void load_all();
    };
}
