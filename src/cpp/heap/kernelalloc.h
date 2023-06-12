#pragma once

#include "stdint.h"

namespace kheap {
    struct BlockMeta {
        uint32_t size;
        bool allocated;
        uint16_t magic;
        BlockMeta* prev;
        BlockMeta* next;
        uint8_t data[];
    };

    static BlockMeta* head_block;
    static BlockMeta* tail_block;

    void init();
};

extern "C" void* kmalloc(uint32_t size, bool reset = false);
extern "C" bool kfree(void* address);
extern "C" void* krealloc(void* address, uint32_t size);
bool kallocated(void* address);