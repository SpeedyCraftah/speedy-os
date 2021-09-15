#include "idt.h"

#include "../panic/panic.h"
#include "../io/video.h"

IDTEntry IDTEntries[256];

IDTEntry idt_define_gate(void (*offset)(), uint8_t attributes) {
    IDTEntry entry;

    entry.Reserved = 0;
    entry.Attributes = attributes;
    entry.Selector = 0x08;

    // Split the god forbidden annoying bits thanks to having to be
    // backwards compatible with 16-bit systems.
    entry.Offset0 = (uint16_t)(((uint32_t)offset) & 0xFFFF);
    entry.Offset1 = (uint16_t)(((uint32_t)offset) >> 16);

    return entry;
}

void HandleInterrupt() {
    kernel::panic("An interrupt has occured: Division 0 by 0 is undefined.", false);
    asm volatile("cli; hlt");
}