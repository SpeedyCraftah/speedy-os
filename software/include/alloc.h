#pragma once

#include "stdint.h"

namespace heap {
    struct BlockMeta {
        uint32_t size;
        bool allocated;
        uint16_t magic;
        BlockMeta* prev;
        BlockMeta* next;
        uint8_t data[];
    };

    static BlockMeta* head_block = reinterpret_cast<BlockMeta*>(314572800);
    //static BlockMeta* tail_block;

    void init();
};

void* malloc(uint32_t size, bool reset = false);
bool free(void* address);
void* realloc(void* address, uint32_t size);
bool allocated(void* address);