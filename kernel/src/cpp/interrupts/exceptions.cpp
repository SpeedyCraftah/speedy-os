#include "exceptions.h"

#include <stdint.h>
#include "../tables/idt.h"
#include "../io/video.h"
#include "../../../../shared/conversions.h"
#include "interrupts.h"

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

    IDTEntries[0] = idt_define_gate(INTERRUPT_0, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[1] = idt_define_gate(INTERRUPT_1, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[3] = idt_define_gate(INTERRUPT_3, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[4] = idt_define_gate(INTERRUPT_4, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[5] = idt_define_gate(INTERRUPT_5, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[6] = idt_define_gate(INTERRUPT_6, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[7] = idt_define_gate(INTERRUPT_7, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[8] = idt_define_gate(INTERRUPT_8, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[10] = idt_define_gate(INTERRUPT_10, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[11] = idt_define_gate(INTERRUPT_11, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[12] = idt_define_gate(INTERRUPT_12, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[13] = idt_define_gate(INTERRUPT_13, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[14] = idt_define_gate(INTERRUPT_14, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[16] = idt_define_gate(INTERRUPT_16, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[17] = idt_define_gate(INTERRUPT_17, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[18] = idt_define_gate(INTERRUPT_18, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[19] = idt_define_gate(INTERRUPT_19, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[20] = idt_define_gate(INTERRUPT_20, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
    IDTEntries[30] = idt_define_gate(INTERRUPT_30, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_0);
}

// High level interrupt handler for CPU exceptions.
// Also has fastcall attribute for extra performance & ease.
// (vector passed via register).
extern "C" __attribute__((fastcall)) void HandleGeneralCPUException(uint8_t vector) {
    // General Protection Fault (will soon kernel panic).
    if (vector == 13) {
        video::printf("\n\nA general protection fault has occurred.\n", VGA_COLOUR::LIGHT_RED);
        video::printf("System has been halted.", VGA_COLOUR::LIGHT_BLUE);

        asm volatile("cli; hlt");
    }

    video::printnl();
    video::printf("Exception Interrupt No. ");
    video::printf(conversions::s_int_to_char(vector));
    
    // Debug interrupt.
    if (vector != 1) {
        video::printf("\nSystem has been halted.", VGA_COLOUR::LIGHT_BLUE);
    
        asm volatile("cli; hlt");
    }
}
