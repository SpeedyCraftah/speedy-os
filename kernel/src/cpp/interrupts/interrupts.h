#pragma once

enum INT_GateType {
    TASK = 0b0101,
    INTERRUPT = 0b1110,
    TRAP = 0b1111
};

enum INT_IOPL {
    RING_0 = 0b00 << 5,
    RING_1 = 0b01 << 5,
    RING_2 = 0b10 << 5,
    RING_3 = 0b11 << 5
};

#define INT_Present 1 << 7