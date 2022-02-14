#include "random.h"

void random::set_seed(uint32_t seed) {
    entropy_value = (seed * 32853) % 2187;
}

void random::add_entropy(uint32_t value) {
    value = (value * 32853) % 2187;

    entropy_value *= value;
}

uint32_t random::next() {
    uint32_t result = entropy_value;

    add_entropy((result * 2196) * (entropy_value << (entropy_value % 10)));

    return result;
}
