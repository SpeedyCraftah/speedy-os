#pragma once

#include "stdint.h"
#include "../../structures/linked_array.h"

struct SteadyDataSink {
    struct DataFragment {
        uint8_t data;
        uint32_t size;
    };

    uint32_t handle_id;
    structures::linked_array<DataFragment> fragments;

    // TODO - check if is implicitly constructed/destructed 
    void destroy() {
        fragments.~linked_array();
    }
};