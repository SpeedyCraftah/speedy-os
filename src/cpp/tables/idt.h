#pragma once

#include <stdint.h>

struct IDTDescriptor {
    uint16_t Limit;
    uint32_t Base;
} __attribute__((packed));

struct IDTEntry {
    unsigned Offset0 : 16;
    unsigned Selector : 16;
    unsigned Reserved : 8;
    unsigned Attributes : 8;
    unsigned Offset1 : 16;
} __attribute__((packed));

// Align for better performance.
extern IDTEntry IDTEntries[256] __attribute__((aligned(0x10)));

IDTEntry idt_define_gate(void (*offset)(), uint8_t attributes);

extern "C" void LoadIDT(IDTDescriptor* descriptor);