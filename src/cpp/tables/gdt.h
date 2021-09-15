#pragma once

#include "stdint.h"

struct GDTDescriptor {
    uint16_t Size;
    uint32_t Offset;
} __attribute__((packed));

struct GDTEntry {
    unsigned Limit0 : 16;
    unsigned Base0 : 16;
    unsigned Base1 : 8;
    unsigned AccessByte : 8;
    unsigned Limit1 : 4;
    unsigned Flags : 4;
    unsigned Base2 : 8;
} __attribute__((packed));

struct GDT {
    GDTEntry Null;
    GDTEntry KernelCode;
    GDTEntry KernelData;
    GDTEntry UserCode;
    GDTEntry UserData;
} __attribute__((packed))
__attribute((aligned(0x1000)));

extern GDT DefaultGDT;

extern "C" void LoadGDT(GDTDescriptor* gdtDescriptor);