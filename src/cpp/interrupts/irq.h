#pragma once

#include <stdint.h>
#include "../structures/events.h"

namespace interrupts {
    class irq {
        public:
            static void load_all();
    };
}
