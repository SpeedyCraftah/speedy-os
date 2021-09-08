// Contains critical code to handle the jump
// from low level to high level code.
// At this point the OS is ready to hand over control
// to the kernel.

#include <stdint.h>
#include "../cpp/kernel.h"

typedef void (*constructor)();
extern "C" constructor* start_ctors;
extern "C" constructor* end_ctors;
extern "C" void callConstructors()
{
    for (constructor* i = start_ctors; i != end_ctors; i++) {
        (*i)();
    }
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber)
{
    // Final polishing.
    asm("call disable_vga_cursor");
    asm("call fix_blinking_text");

    kernelControlHandOver();
}
