#include "irq.h"

#include <stdint.h>
#include "../tables/idt.h"
#include "../io/video.h"
#include "../abstractions/io_port.h"
#include "../structures/string.h"

#include "../drivers/keyboard/keyboard.h"

// Master PICs.
extern "C" void INTERRUPT_33();
extern "C" void INTERRUPT_34();
extern "C" void INTERRUPT_35();
extern "C" void INTERRUPT_36();
extern "C" void INTERRUPT_37();
extern "C" void INTERRUPT_38();
extern "C" void INTERRUPT_39();
extern "C" void INTERRUPT_40();

// Slave PICs.
extern "C" void INTERRUPT_41();
extern "C" void INTERRUPT_42();
extern "C" void INTERRUPT_43();
extern "C" void INTERRUPT_44();
extern "C" void INTERRUPT_45();
extern "C" void INTERRUPT_46();
extern "C" void INTERRUPT_47();
extern "C" void INTERRUPT_48();

void interrupts::irq::load_all() {
    IDTEntries[32] = idt_define_gate(INTERRUPT_33, 0x8E);
    IDTEntries[33] = idt_define_gate(INTERRUPT_34, 0x8E);
    IDTEntries[34] = idt_define_gate(INTERRUPT_35, 0x8E);
    IDTEntries[35] = idt_define_gate(INTERRUPT_36, 0x8E);
    IDTEntries[36] = idt_define_gate(INTERRUPT_37, 0x8E);
    IDTEntries[37] = idt_define_gate(INTERRUPT_38, 0x8E);
    IDTEntries[38] = idt_define_gate(INTERRUPT_39, 0x8E);
    IDTEntries[39] = idt_define_gate(INTERRUPT_40, 0x8E);

    IDTEntries[40] = idt_define_gate(INTERRUPT_41, 0x8E);
    IDTEntries[41] = idt_define_gate(INTERRUPT_42, 0x8E);
    IDTEntries[42] = idt_define_gate(INTERRUPT_43, 0x8E);
    IDTEntries[43] = idt_define_gate(INTERRUPT_44, 0x8E);
    IDTEntries[44] = idt_define_gate(INTERRUPT_45, 0x8E);
    IDTEntries[45] = idt_define_gate(INTERRUPT_46, 0x8E);
    IDTEntries[46] = idt_define_gate(INTERRUPT_47, 0x8E);
    IDTEntries[47] = idt_define_gate(INTERRUPT_48, 0x8E);
}

static unsigned int counter = 0;

// High level interrupt handler for IRQs
// Also has fastcall attribute for extra performance & ease.
// (vector passed via register).
extern "C" __attribute__((fastcall)) void HandleIRQInterrupt(uint8_t vector) {
    uint8_t irq = vector - 33;

    // PIT interrupt (every 10ms).
    if (irq == 0) {
        counter += 10;
    }

    if (irq == 1) {
        drivers::keyboard::handle_interrupt();
    }

    // Emit.
    // interrupts::irq::events.emit_event(irq, irq);
}
