// Kernel now has all control.

#include <stdint.h>
#include "interrupts/controllers/pit.h"
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
#include "interrupts/exceptions.h"
#include "abstractions/io_port.h"
#include "interrupts/irq.h"
#include "interrupts/controllers/pic.h"
#include "structures/events.h"

#include "drivers/keyboard/keyboard.h"

#include <limits.h>

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
    interrupts::irq::load_all();

    // Remap the PIC offsets to entry 32 & 40.
    controllers::pic::remap(32, 40);

    // Mask 0 for now since the PIT isn't required yet.
    // controllers::pic::mask_line(0);

    // Load the table and enable interrupts.
    LoadIDT(&idtDescriptor);

    // Configure PIT timer to send an interrupt every 10 milliseconds.
    controllers::pit::set_channel_0_frequency(100);

    // Load keyboard driver.
    drivers::keyboard::load();


    // Halt loop to let CPU sleep.
    while(1) {
        asm volatile("hlt");
    };
}
