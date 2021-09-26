#include "fpu.h"

void chips::fpu::set_cw(uint16_t cw) {
    asm volatile("fldcw %0" :: "m"(cw));
}

void chips::fpu::init_fpu() {
    // Move CR4 into a variable.
    uint32_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));

    // Set bits.
	cr4 |= 0x200;

    // Update CR4 register.
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));

    // Set word.
	set_cw(0x37F);
}