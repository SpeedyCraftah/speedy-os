#include "hw_random.h"

namespace hw_random {
    void add_entropy(uint32_t entropy) {
        generator.add_entropy(entropy);
    }

    bool value_available() {
        return true;
    }

    uint32_t get_value() {
        return generator.next();
    }
};