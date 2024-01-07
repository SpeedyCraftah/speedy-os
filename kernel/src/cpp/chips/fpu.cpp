#include "fpu.h"

void chips::fpu::set_cw(uint16_t cw) {
    asm volatile("fldcw %0" :: "m"(cw));
}

void chips::fpu::init_fpu() {
    // Move CR4 into a variable.
    uint32_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));

    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));

    // Set bits.
    cr4 |= 0x200;
    cr4 |= 0x400;

    // Update CR4 register.
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    // Set word.
    // 64-bit precision, handled exceptions, rounds closest, infinity sign ignored.
	set_cw(0x33F);
}