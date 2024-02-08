#include "../../include/sys.h"
#include "../../../shared/conversions.h"
#include "../../../shared/map.h"
#include "../../../shared/flex_array.h"
#include "../../../shared/memory.h"

structures::map<char*> cool_map;
structures::map<char*> cool_map2;

structures::flexible_array<int> cool_flex;

int main() {
    speedyos::speedyshell::printf("running!\n");

    uint32_t* graphics = speedyos::upgrade_graphics();
    graphics[0] = 0xFFFFFFFF;
    graphics[1] = 0xFFFFFFFF;
    graphics[2] = 0xFFFFFFFF;
    graphics[3] = 0xFFFFFFFF;

    return 0;
}