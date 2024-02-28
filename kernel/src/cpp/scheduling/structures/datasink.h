#pragma once

#include "stdint.h"
#include "../../../../../shared/linked_array.h"
#include "../../../../../shared/map.h"
#include "../../misc/assert.h"
#include "../../../../../shared/memory.h"

struct DataSinkPermissions {
    bool owner : 1; // Is the owner of the sink.
    bool read : 1; // Can read data from the sink.
    bool write : 1; // Can write data to the sink.
};

struct SteadyDataSink {
    struct DataFragment {
        uint8_t* data;
        uint8_t* data_ptr;
        uint32_t size_remaining;
        uint32_t actual_size;
        bool unique; // Whether the fragment was allocated and copied for this specific sink or not.
    };

    enum AppendType {
        COPY_BUFFER, TRANSFER_BUFFER_OWNERSHIP, EXISTING_BUFFER
    };

    uint32_t handle_id;
    structures::linked_array<DataFragment> fragments;
    structures::map<DataSinkPermissions> permissions;

    bool append_data(uint8_t* buffer, uint32_t buffer_size, AppendType append_type = AppendType::COPY_BUFFER) {
        if (buffer_size == 0) return false;

        DataFragment fragment;
        fragment.size_remaining = buffer_size;
        fragment.actual_size = buffer_size;

        if (append_type == AppendType::COPY_BUFFER) {
            fragment.data = (uint8_t*)kmalloc(buffer_size);
            fragment.unique = true;
            memcpy(fragment.data, buffer, buffer_size);
        }

        else if (append_type == AppendType::TRANSFER_BUFFER_OWNERSHIP) {
            fragment.data = buffer;
            fragment.unique = true;
        }

        else if (append_type == AppendType::EXISTING_BUFFER) {
            fragment.data = buffer;
            fragment.unique = false;
        }

        fragment.data_ptr = fragment.data;

        this->fragments.push(fragment);

        return true;
    }

    uint32_t consume_data_stream(uint8_t* destination, uint32_t consume_size) {
        uint32_t read_bytes = 0;
        while (consume_size != 0 && this->fragments.get_size() != 0) {
            DataFragment& fragment = this->fragments.peek_front();
            
            // Entire fragment to be removed.
            if (consume_size >= fragment.size_remaining) {
                memcpy(destination, fragment.data_ptr, fragment.size_remaining);
                if (fragment.unique) kfree(fragment.data);
                consume_size -= fragment.size_remaining;
                destination += fragment.size_remaining;
                read_bytes += fragment.size_remaining;
                this->fragments.shift();
            }

            // Partial fragment read.
            else {
                memcpy(destination, fragment.data_ptr, consume_size);
                fragment.size_remaining -= consume_size;
                fragment.data_ptr += consume_size;
                read_bytes += consume_size;
                break;
            }
        }

        return read_bytes;
    }

    bool consume_block() {
        if (!this->fragments.get_size()) return false;
        
        DataFragment fragment = this->fragments.shift();
        if (fragment.unique) kfree(fragment.data);

        return true;
    }
};