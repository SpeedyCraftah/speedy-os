#include "software.h"

#include <stdint.h>
#include "../tables/idt.h"
#include "interrupts.h"

extern "C" void INTERRUPT_128();

void interrupts::software::load_all() {
  IDTEntries[128] = idt_define_gate(INTERRUPT_128, INT_Present | INT_GateType::INTERRUPT | INT_IOPL::RING_3);
}