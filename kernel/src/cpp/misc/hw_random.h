// A more random and hardware-based random number generator which adds entropy from a couple of unpredictable sources.
// Likely not secure since I'm not a cryptographer.

#pragma once

#include "../../../../shared/random.h"

namespace hw_random {
    static random generator;

    void add_entropy(uint32_t entropy);

    // Check if there is enough entropy for a relatively random value.
    bool value_available();

    uint32_t get_value();
};