#pragma once

#include "stdint.h"

struct Registers {
    uint32_t eax = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;
    uint32_t ebx = 0;
    uint32_t esp = 0;
    uint32_t ebp = 0;
    uint32_t esi = 0;
    uint32_t edi = 0;
    uint32_t eflags = 0;

    // cannot be modified or read. only here for the sake of cleanliness.
    uint32_t eip = 0;
};