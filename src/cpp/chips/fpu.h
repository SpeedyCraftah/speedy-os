// Float Processing Unit - Used to enable the FPU and add support for floats.

#pragma once

#include <stdint.h>

namespace chips {
    namespace fpu {
        void set_cw(uint16_t cw);
        void init_fpu();   
    }
}