#pragma once

#include "scheduler.h"

struct InterruptFrame {
    uint32_t EIP;
    uint16_t CS;
    uint16_t _CS_PADDING;
    uint32_t EFLAGS;
}__attribute__((packed));