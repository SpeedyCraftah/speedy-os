#pragma once

#include "stdint.h"

struct Registers {
    uint32_t eax = 0; // 0
    uint32_t ecx = 0; // 4
    uint32_t edx = 0; // 8
    uint32_t ebx = 0; // 12
    uint32_t esp = 0; // 16
    uint32_t ebp = 0; // 20
    uint32_t esi = 0; // 24
    uint32_t edi = 0; // 28
    uint32_t eip = 0; // 32
    uint8_t flags = 0; // 36
    char fpu_state[108]; // 37
};