// Kernel now has all control.

#include "chips/fpu.h"
#include "chips/pit.h"

#include "structures/string.h"

#include "tables/gdt.h"
#include "tables/idt.h"

#include "interrupts/exceptions.h"
#include "interrupts/software.h"
#include "interrupts/irq.h"

#include "chips/pic.h"

#include "drivers/keyboard/keyboard.h"
#include "scheduling/scheduler.h"

#include "programs/test.h"

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
    interrupts::software::load_all();

    // Configure FPU to be able to run float operations.
    chips::fpu::init_fpu();

    // Configure PIT timer to send an interrupt every 5 milliseconds (around 200 hertz).
    // Set to this amount due to bugs and issues with virtual machines keeping up.
    // Will be adjusted eventually.
    chips::pit::set_channel_0_frequency(200);

    // Remap the PIC offsets to entry 32 & 40.
    chips::pic::remap(32, 40);

    // Mask PIT since the scheduler isn't ready yet.
    chips::pic::mask_line(0);

    // Load the table and enable interrupts.
    LoadIDT(&idtDescriptor);

    // Initialise scheduler.
    scheduler::initialise();

    // START OF LOADING DRIVERS.

    // Load keyboard driver.
    drivers::keyboard::load();

    // END OF LOADING DRIVERS.

    // Start a test process.
    scheduler::start_process(
        structures::string("test"), 
        TestProgramCPP::main, 
        TaskStatus::RUNNING, 0, true
    );
    
    // Unmask the PIT as scheduler is now ready.
    chips::pic::unmask_line(0);

    // Wait for first tick of the PIT after which control will be handed to the scheduler.
    asm volatile("hlt");
}
