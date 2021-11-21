#pragma once

#include "stdint.h"

namespace heap {
    struct record {
        uint32_t process_id;
        uint32_t size;
        uint8_t* location;
        bool reserved;
    } __attribute__((packed));

    // Define location of meta data array.
    static record* data_meta = (record*)0x01000000;

    // Statistics.
    static uint32_t reserved_blocks = 0;
    static uint32_t total_blocks = 0;
    static uint32_t total_reserved_bytes = 0;

    // Location of heap data.
    static uint8_t* next_data_location = (uint8_t*)0x1200000;

    // Functions.
    void* malloc(uint32_t size, bool reset = true, bool skip_reuse = false, uint32_t process_id = 0);
    bool free(void* ptr);

    void free_by_process_id(uint32_t process_id);

    uint32_t get_allocated_bytes();
    uint32_t get_total_blocks();
    uint32_t get_reserved_blocks();
}