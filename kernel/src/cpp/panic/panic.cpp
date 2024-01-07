#include "panic.h"

#include "../io/video.h"

void kernel::panic(char* error, bool hang) {
    video::clearscr(VGA_COLOUR::BLUE);
    
    video::printf("KERNEL PANIC\n------------\n\n", VGA_COLOUR::WHITE);
    video::printf("A kernel panic has occured.\n\n", VGA_COLOUR::WHITE);
    video::printf("SpeedyOS has detected a serious problem and has shut itself down to prevent\nfurther undefined functionality.\n\n\n", VGA_COLOUR::WHITE);
    video::printf("PANIC DETAILS: ", VGA_COLOUR::WHITE);
    video::printf(error, VGA_COLOUR::LIGHT_GREY);

    if (hang) {
        asm volatile("cli; hlt");
        __builtin_unreachable();
    }
}