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

void kernelControlHandOver() {
    video::printnl();
    video::printf("Welcome!\n\n", VGA_COLOUR::LIGHT_BLUE);

    structures::flexible_array<int> array;

    array.push(1);
    
    array.set_at(1, 2);
    array.set_at(3, 3);

    video::printnl();
    video::printf(conversions::s_int_to_char(array.get_capacity()));
    video::printnl();

    array.set_at(5, 4);
    
    /*for (int i = 0; i < 8; i++) {
        video::printf(conversions::s_int_to_char(array[i]));
        video::printnl();
    }*/

    array.defragment();

    array.resize(array.get_size());

    array.push(5);

    video::printnl();
    video::printf(conversions::s_int_to_char(array.get_capacity()));
    video::printnl();

    video::printnl();

    /*for (int i = 0; i < 8; i++) {
        video::printf(conversions::s_int_to_char(array[i]));
        video::printnl();
    }*/

    video::printf("\nWayyyyy", VGA_COLOUR::LIGHT_RED);
    video::printnl();

    asm("hlt");
}
