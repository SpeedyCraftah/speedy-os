#include "stdint.h"

namespace acpi {
    enum sleep_type {
        S0_NONE = 0,
        S1_REDUCED = 1,
        S5_MECHANICALOFF = 5,
    };

    void initialise();
    void enter_sleep_state(sleep_type type);
}