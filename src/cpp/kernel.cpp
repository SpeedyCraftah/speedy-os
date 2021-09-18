// Kernel now has all control.

#include <stdint.h>
#include "io/video.h"
#include "misc/conversions.h"
#include "heap/allocator.h"
#include "misc/str.h"
#include "abstractions/cpu.h"
#include "panic/panic.h"
#include "structures/map.h"
#include "misc/smart_ptr.h"
#include "structures/string.h"
#include "structures/flex_array.h"

#include "tables/gdt.h"
#include "tables/idt.h"
#include "interrupts/general.h"
#include "abstractions/io_port.h"
#include "interrupts/irq.h"
#include "interrupts/controllers/pic.h"
#include "structures/events.h"

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

    // Loading of gates seperated into another file due to size (and complexity).
    interrupts::exceptions::load_all();
    interrupts::irqs::load_all();

    // Remap the PIC offsets to entry 32 & 40.
    controllers::pic::remap(32, 40);

    // Mask 0 for now since the PIT appears to be misconfigured and we do not need a tick yet.
    controllers::pic::mask_line(0);

    // Load the table and enable interrupts.
    LoadIDT(&idtDescriptor);


    // Disable interrupts and halt.
    // asm volatile("cli; hlt");

    while(1) {};
}
