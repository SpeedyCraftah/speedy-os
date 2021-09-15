// Kernel now has all control.

#include <stdint.h>
#include "io/video.h"
#include "misc/conversions.h"
#include "heap/allocator.h"
#include "misc/str.h"
#include "cpu/registers.h"
#include "panic/panic.h"
#include "structures/map.h"
#include "misc/smart_ptr.h"
#include "structures/string.h"
#include "structures/flex_array.h"

#include "tables/gdt.h"
#include "tables/idt.h"

extern "C" void OnInterrupt();

void kernelControlHandOver() {
    // Clear screen for QEMU.
    video::clearscr();

    video::printnl();
    video::printf("Welcome.\n\n", VGA_COLOUR::LIGHT_GREEN);

    // Load the global descriptor table.
    // Defines memory segments.
    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint32_t)&DefaultGDT;

    LoadGDT(&gdtDescriptor);


    // Loads the interrupt descriptor table.
    // Defines interrupts.
    IDTDescriptor idtDescriptor;
    idtDescriptor.Limit = sizeof(IDTEntry) * 256 - 1;
    idtDescriptor.Base = (uint32_t)&IDTEntries;

    // Define test gate (0/0 fault).
    IDTEntries[0] = idt_define_gate(OnInterrupt, 0x8E);

    LoadIDT(&idtDescriptor);

    // Trigger 0 / 0 interrupt.
    volatile int test = 0 / 0;

    asm volatile("hlt");
}
