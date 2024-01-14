#include "../../include/sys.h"
#include "../../include/misc/conversions.h"
#include "../../include/structures/map.h"
#include "../../include/structures/flex_array.h"
#include "../../include/misc/memory.h"

structures::map<char*> cool_map;
structures::map<char*> cool_map2;

structures::flexible_array<int> cool_flex;

int main() {
    speedyos::speedyshell::printf(conversions::u_int_to_char(cool_flex.get_capacity() + cool_map.get_capacity() + cool_map2.get_capacity()));

    speedyos::speedyshell::printf("hello world!\n");
    speedyos::speedyshell::printf("everything is great\n", (speedyos::VGAColour)0x00ff00);
    speedyos::speedyshell::printf("or is it???\n", (speedyos::VGAColour)0xff0000);

    speedyos::alloc_virtual_pages((void*)2147483648, 1, speedyos::PageAllocationFlag::ALLOCATE_ON_ACCESS);
    *(int*)2147483648 = 2;

    speedyos::speedyshell::printf("\nit is :)\n", (speedyos::VGAColour)0x00ff00);

    /*while (true) {
    }*/

    char buf[10];
    memset(buf, 10, 20);

    return 0;
}