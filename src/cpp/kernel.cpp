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

extern "C" void TestInt();

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

    // UNSTABLE CODE BELOW (?)

    // Loads the interrupt descriptor table.
    // Defines interrupts.
    IDTDescriptor idtDescriptor;
    idtDescriptor.Limit = sizeof(IDTEntry) * 256 - 1;
    idtDescriptor.Base = (uint32_t)&IDTEntries;

    // Loading of gates seperated into another file due to size (and complexity).
    GeneralInterruptManager::LoadAll();

    LoadIDT(&idtDescriptor);

    // Trigger some test interrupts to test
    asm volatile("int $1");
    asm volatile("int $3");

    // Crash on purpose (GPF).
    asm volatile("int $2");

    // Disable interrupts and halt.
    asm volatile("cli; hlt");
}
