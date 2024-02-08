#include "../../include/sys.h"
#include "../../../shared/conversions.h"
#include "../../../shared/map.h"
#include "../../../shared/flex_array.h"
#include "../../../shared/memory.h"
#include "../../../shared/graphics/graphics.h"

structures::map<char*> cool_map;
structures::map<char*> cool_map2;

structures::flexible_array<int> cool_flex;

int main() {
    speedyos::speedyshell::printf("running!\n");

    uint32_t* graphics_ptr = speedyos::upgrade_graphics();
    graphics::init(graphics_ptr, speedyos::fetch_graphics_resolution(), speedyos::fetch_colour_depth());

    return 0;
}