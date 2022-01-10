#pragma once

#include "stdint.h"

namespace random {
    uint32_t next();
    void set_seed(uint32_t seed);
    void add_entropy(uint32_t value);
}