#include "exceptions.h"

#include <stdint.h>
#include "../tables/idt.h"
#include "../io/video.h"

// Required to access memory locations of labels.
extern "C" void INTERRUPT_0();
extern "C" void INTERRUPT_1();
extern "C" void INTERRUPT_3();
extern "C" void INTERRUPT_4();
extern "C" void INTERRUPT_5();
extern "C" void INTERRUPT_6();
extern "C" void INTERRUPT_7();
extern "C" void INTERRUPT_8();
extern "C" void INTERRUPT_10();
extern "C" void INTERRUPT_11();
extern "C" void INTERRUPT_12();
extern "C" void INTERRUPT_13();
extern "C" void INTERRUPT_14();
extern "C" void INTERRUPT_16();
extern "C" void INTERRUPT_17();
extern "C" void INTERRUPT_18();
extern "C" void INTERRUPT_19();
extern "C" void INTERRUPT_20();
extern "C" void INTERRUPT_30();

// Loads all CPU exceptions (except excluded ones).
void interrupts::exceptions::load_all() {
    // Don't try to tell me this is tedious.

    IDTEntries[0] = idt_define_gate(INTERRUPT_0, 0x8E);
    IDTEntries[1] = idt_define_gate(INTERRUPT_1, 0x8E);
    IDTEntries[3] = idt_define_gate(INTERRUPT_3, 0x8E);
    IDTEntries[4] = idt_define_gate(INTERRUPT_4, 0x8E);
    IDTEntries[5] = idt_define_gate(INTERRUPT_5, 0x8E);
    IDTEntries[6] = idt_define_gate(INTERRUPT_6, 0x8E);
    IDTEntries[7] = idt_define_gate(INTERRUPT_7, 0x8E);
    IDTEntries[8] = idt_define_gate(INTERRUPT_8, 0x8E);
    IDTEntries[10] = idt_define_gate(INTERRUPT_10, 0x8E);
    IDTEntries[11] = idt_define_gate(INTERRUPT_11, 0x8E);
    IDTEntries[12] = idt_define_gate(INTERRUPT_12, 0x8E);
    IDTEntries[13] = idt_define_gate(INTERRUPT_13, 0x8E);
    IDTEntries[14] = idt_define_gate(INTERRUPT_14, 0x8E);
    IDTEntries[16] = idt_define_gate(INTERRUPT_16, 0x8E);
    IDTEntries[17] = idt_define_gate(INTERRUPT_17, 0x8E);
    IDTEntries[18] = idt_define_gate(INTERRUPT_18, 0x8E);
    IDTEntries[19] = idt_define_gate(INTERRUPT_19, 0x8E);
    IDTEntries[20] = idt_define_gate(INTERRUPT_20, 0x8E);
    IDTEntries[30] = idt_define_gate(INTERRUPT_30, 0x8E);
}

// High level interrupt handler for CPU exceptions.
// Also has fastcall attribute for extra performance & ease.
// (vector passed via register).
extern "C" __attribute__((fastcall)) void HandleGeneralCPUException(uint8_t vector) {
    // General Protection Fault (will soon kernel panic).
    if (vector == 13) {
        video::printf("\n\nGENERAL PROTECTION FAULT INTERRUPT\n", VGA_COLOUR::LIGHT_RED);
        video::printf("SYSTEM HAS BEEN HALTED", VGA_COLOUR::LIGHT_BLUE);

        asm volatile("cli; hlt");
    }

    // Me using the old method instead of using my string function.
    video::printf("Interrupt No. ");
    video::printf(conversions::s_int_to_char(vector));
    video::printnl();
}
