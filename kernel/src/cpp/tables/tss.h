#pragma once

#include "stdint.h"

struct TSS {
    uint32_t link;
    uint32_t esp0; // Stack pointer for kernel mode.
    uint32_t ss0; // Stack segment for kernel mode.
    char _padding_unused[96];
} __attribute__((packed));

extern TSS DefaultTSS;

void tss_setup_default();