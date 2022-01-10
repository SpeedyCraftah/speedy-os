#include "random.h"

static uint32_t entropy_value = 2312255697;

namespace random {
    void set_seed(uint32_t seed) {
        entropy_value = (seed * 32853) % 2187;
    }

    void add_entropy(uint32_t value) {
        value = (value * 32853) % 2187;

        entropy_value += value;
    }

    uint32_t next() {
        uint32_t result = entropy_value;

        add_entropy(result * 2196);

        return result;
    }
}