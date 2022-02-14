#pragma once

#include "stdint.h"

namespace random {
    public:    
        uint32_t next();
        void set_seed(uint32_t seed);
        void add_entropy(uint32_t value);
    
    private:
        uint32_t entropy_value = 2312255697;
}
