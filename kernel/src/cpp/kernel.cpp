// Kernel now has all control.

#include "kernel.h"

#include "abstractions/io_port.h"
#include "chips/fpu.h"
#include "chips/pit.h"

#include "io/video.h"
#include "misc/conversions.h"
#include "scheduling/structures/process.h"
//#include "software/include/sys.h"
#include "structures/string.h"

#include "tables/gdt.h"
#include "tables/idt.h"

#include "interrupts/exceptions.h"
#include "interrupts/software.h"
#include "interrupts/irq.h"

#include "chips/pic.h"

#include "drivers/keyboard/keyboard.h"
#include "drivers/mouse/mouse.h"
#include "scheduling/scheduler.h"

#include "software/system/speedyshell/main.h"

#include "stdint.h"
#include "io/graphics.h"
#include "software/system/speedyshell/main.h"
//#include "software/system/cpu_usage/main.h"
#include "misc/math.h"
#include "misc/memory.h"
#include "stdarg.h"
#include "acpi/acpi.h"
//#include "software/system/tsccalc/main.h"

#include "paging/paging.h"
#include "paging/virtual.h"
#include "heap/kernelalloc.h"
#include "heap/physical.h"

#include "loader/elf.h"
#include "tables/tss.h"


bool isTerminalInterface = false;

extern "C" uint8_t* structure_address = 0;

extern "C" void debug();

extern "C" __attribute__((fastcall)) void enable_paging_asm(PageDirectory* address);
extern "C" void callConstructors();

void kernelControlHandOver() {
    // Clear screen for QEMU.
    //video::clearscr();

    uint8_t* addr = structure_address + 88;
    uint32_t loc1 = *(uint32_t*)addr; 
    graphics::video_address = (uint32_t*)reinterpret_cast<uint32_t*>(loc1);
    graphics::resolution_width = *(uint32_t*)(structure_address + 100);
    graphics::resolution_height = *(uint32_t*)(structure_address + 104);

    // Initialise the page allocator and heap.
    physical_allocator::init();
    kheap::init();

    // Calls constructors which is necessary for classes to work and for logs to work.
    //callConstructors();

    uint32_t mod_count = *(uint32_t*)(structure_address + 20);
    uint32_t* mod_addr_first = reinterpret_cast<uint32_t*>(*(uint32_t*)(structure_address + 24));

    video::printf(*(char**)(structure_address + 64));
    video::printnl();
    video::printf("Found ");
    video::printf(conversions::u_int_to_char(mod_count));
    video::printf(" modules\n");
    
    for (int i = 0; i < mod_count; i++) {
        uint32_t* mod_addr = mod_addr_first + (i * 4);
        video::printf("Module ");
        video::printf(conversions::s_int_to_char(i + 1));
        video::printf(" name: ");
        video::printf(*(char**)(mod_addr + 2));
        video::printnl();
        video::printf("Module start: ");
        video::printf(conversions::u_int_to_char(*mod_addr));
        video::printnl();
        video::printf("Module end: ");
        video::printf(conversions::u_int_to_char(*(mod_addr + 1)));
        video::printf("\nModule is ");
        video::printf(conversions::u_int_to_char(*(mod_addr + 1) - *mod_addr));
        video::printf(" bytes long\n");
        video::printf("Contents: ");
        video::printf(*(char**)(mod_addr));
        video::printnl();
        video::printnl();
    }

    video::printf_log("Kernel", "Initialised physical page allocator..");
    video::printf_log("Kernel", "Initialised kernel allocator..");
    video::printf_log("Kernel", "Initialised kernel constructors..");

    video::current_address = graphics::video_address;
    video::address = graphics::video_address;

    video::printf_log("Kernel", "Defining GDT memory segments (4G)...");

    // Load the global descriptor table.
    
    // Defines memory segments.
    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint32_t)&DefaultGDT;

    LoadGDT(&gdtDescriptor);

    video::printf_log("Kernel", "Defining and loading TSS...");

    // Load and define the TSS.
    tss_setup_default();

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

    // Mask FPU line.
    video::printf_log("Kernel", "Configuring PIC...");

    // Remap the PIC offsets to entry 32 & 40.
    chips::pic::remap(32, 40);

    // Mask PIT since the scheduler isn't ready yet.
    chips::pic::mask_line(0);

    video::printf_log("Kernel", "Initialising ACPI (this may take a while)...");
    //acpi::initialise();

    video::printf_log("Kernel", "Initialising IDT and enabling interrupts...");

    // Load the table and enable interrupts.
    LoadIDT(&idtDescriptor);

    video::printf_log("Kernel", "Initialising task scheduler...");

    // Initialise scheduler.
    scheduler::initialise();

    video::printf_log("Kernel", "Enabling paging...");
    paging::enable(paging::kernel_page_directory);
    
    video::printf_log("Kernel", "Setting PIT timer frequency to 200Hz...");

    // Configure PIT timer to send an interrupt every 5 milliseconds.
    chips::pit::set_channel_0_frequency(200);

    // START OF LOADING DRIVERS.

    video::printf_log("Kernel", "Loading keyboard driver...");

    // Load keyboard driver.
    drivers::keyboard::load();

    video::printf_log("Kernel", "Loading mouse driver...");

    // Load mouse driver.
    drivers::mouse::load();

    // END OF LOADING DRIVERS.

    video::printf_log("Kernel", "Selecting SpeedyShell as interfacing method...");

    isTerminalInterface = true;

    video::printf_log("Kernel", "Preparing SpeedyShell...");

    video::printf_log("Kernel", "Starting TSC calculator process...");

    ProcessFlags flags;
    flags.system_process = false;

    Process* p = loader::load_elf32_executable_as_process(*(char**)(mod_addr_first + 2), flags, reinterpret_cast<void*>(*mod_addr_first), reinterpret_cast<void*>(*(mod_addr_first + 1)));

    //Process* p = scheduler::create_process("test", testp);
    
    // Start shell.
    /*scheduler::create_process(
        "TSC Calculator", 
        software::tsc_calc::start,
        flags
    );*/
    // Unmask PIC.
    chips::pic::unmask_line(0);

    // Wait for first tick of the PIT after which control will be handed to the scheduler.
    while (true) {
        asm volatile("hlt");
    }
    __builtin_unreachable();
}
