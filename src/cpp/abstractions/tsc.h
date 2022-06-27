#pragma once

#include "stdint.h"

inline uint64_t fetch_tsc_ticks() {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
