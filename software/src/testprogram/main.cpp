#include "../../include/sys.h"
#include "../../include/misc/conversions.h"
#include "../../include/structures/map.h"

extern "C" volatile int debug_vb = 0;
uint8_t* ptr = (uint8_t*)2147483648;

structures::map<char*> cool_map;

int main() {
    speedyos::speedyshell::printf(conversions::u_int_to_char((cool_map.get_capacity())));

    speedyos::speedyshell::printf("hello world!\n");
    speedyos::speedyshell::printf("everything is great\n", (speedyos::VGAColour)0x00ff00);
    speedyos::speedyshell::printf("or is it???\n", (speedyos::VGAColour)0xff0000);

    speedyos::alloc_virtual_pages((void*)2147483648, 1, speedyos::PageAllocationFlag::ALLOCATE_IMMEDIATELY);

    debug_vb = 69;
    *ptr = 69;
    speedyos::speedyshell::printf(conversions::u_int_to_char(*ptr));
    speedyos::speedyshell::printf("\nit is :)\n", (speedyos::VGAColour)0x00ff00);

    debug_vb = 55;

    while (true) {
        debug_vb++;
    }

    return 0;
}