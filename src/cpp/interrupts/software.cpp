#include "software.h"

#include <stdint.h>
#include "../tables/idt.h"

extern "C" void INTERRUPT_128();

void interrupts::software::load_all() {
  IDTEntries[128] = idt_define_gate(INTERRUPT_128, 0x8E);
}