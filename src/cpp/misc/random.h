#pragma once

#include "stdint.h"

namespace random {
    uint32_t next();
    void add_entropy(uint32_t value);
}