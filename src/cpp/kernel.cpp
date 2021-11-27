// Kernel now has all control.

#include "kernel.h"

#include "chips/fpu.h"
#include "chips/pit.h"

#include "misc/conversions.h"
#include "structures/string.h"

#include "tables/gdt.h"
#include "tables/idt.h"

#include "interrupts/exceptions.h"
#include "interrupts/software.h"
#include "interrupts/irq.h"

#include "chips/pic.h"

#include "drivers/keyboard/keyboard.h"
#include "scheduling/scheduler.h"

#include "software/system/speedyshell/main.h"

void kernelControlHandOver() {
    // Clear screen for QEMU.
    video::clearscr();

    video::printf_log("Kernel", "Defining GDT memory segments (128M)...");

    // Load the global descriptor table.
    // Defines memory segments.
    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint32_t)&DefaultGDT;

    LoadGDT(&gdtDescriptor);

    video::printf_log("Kernel", "Defining IDT interrupt entries...");

    // Loads the interrupt descriptor table.
    // Defines interrupts.
    IDTDescriptor idtDescriptor;
    idtDescriptor.Limit = sizeof(IDTEntry) * 256 - 1;
    idtDescriptor.Base = (uint32_t)&IDTEntries;

    video::printf_log("Kernel", "Populating IDT table...");

    // Loading of gates seperated into another file due to size (and complexity).
    interrupts::exceptions::load_all();
    interrupts::irq::load_all();
    interrupts::software::load_all();

    video::printf_log("Kernel", "Initialising FPU...");

    // Configure FPU to be able to run float operations.
    chips::fpu::init_fpu();

    video::printf_log("Kernel", "Setting PIT timer frequency to 500Hz...");

    // Configure PIT timer to send an interrupt every 2 milliseconds.
    // Will eventually be increased to 1000 hertz (1ms/int).
    chips::pit::set_channel_0_frequency(500);
    
    video::printf_log("Kernel", "Configuring PIC...");

    // Remap the PIC offsets to entry 32 & 40.
    chips::pic::remap(32, 40);

    // Mask PIT since the scheduler isn't ready yet.
    chips::pic::mask_line(0);

    video::printf_log("Kernel", "Initialising IDT and enabling interrupts...");

    // Load the table and enable interrupts.
    LoadIDT(&idtDescriptor);

    video::printf_log("Kernel", "Initialising task scheduler...");

    // Initialise scheduler.
    scheduler::initialise();

    // START OF LOADING DRIVERS.

    video::printf_log("Kernel", "Loading keyboard driver...");

    // Load keyboard driver.
    drivers::keyboard::load();

    // END OF LOADING DRIVERS.

    video::printf_log("Kernel", "Selecting SpeedyShell as interfacing method...");

    isTerminalInterface = true;

    video::printf_log("Kernel", "Preparing SpeedyShell...");

    // Start shell.
    scheduler::start_process(
        structures::string("SpeedyShell"), 
        speedyshell::start,
        TaskStatus::RUNNING,
        ProcessFlag::SYSTEM_PROCESS | ProcessFlag::INTERFACE_PROVIDER,
        true, true
    );

    video::printf_log("Kernel", "Enabling scheduler (PIT)...");
    video::printnl();

    // Unmask the PIT as scheduler is now ready.
    chips::pic::unmask_line(0);

    // Wait for first tick of the PIT after which control will be handed to the scheduler.
    while (true) {
        asm volatile("hlt");
    }
}
