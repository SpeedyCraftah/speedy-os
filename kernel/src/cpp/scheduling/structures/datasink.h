#pragma once

#include "stdint.h"
#include "../../structures/linked_array.h"
#include "../../misc/assert.h"
#include "../../misc/memory.h"

struct SteadyDataSink {
    struct DataFragment {
        uint8_t* data;
        uint32_t size;
        bool unique; // Whether the fragment was allocated and copied for this specific sink or not.
    };

    enum AppendType {
        COPY_BUFFER, TRANSFER_BUFFER_OWNERSHIP, EXISTING_BUFFER
    };

    uint32_t handle_id;
    structures::linked_array<DataFragment> fragments;
    uint32_t actual_size = 0;

    bool append_data(uint8_t* buffer, uint32_t buffer_size, AppendType append_type = AppendType::COPY_BUFFER) {
        DataFragment fragment;
        fragment.size = buffer_size;

        if (append_type == AppendType::COPY_BUFFER) {
            fragment.data = (uint8_t*)kmalloc(buffer_size);
            fragment.unique = true;
            memcpy(fragment.data, buffer, buffer_size);
            this->actual_size += buffer_size;
        }

        else if (append_type == AppendType::TRANSFER_BUFFER_OWNERSHIP) {
            fragment.data = buffer;
            fragment.unique = true;
            this->actual_size += buffer_size;
        }

        else if (append_type == AppendType::EXISTING_BUFFER) {
            fragment.data = buffer;
            fragment.unique = false;
            this->actual_size += buffer_size;
        }

        this->fragments.push(fragment);

        return true;
    }

    
};