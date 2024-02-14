#include "modules.h"

#include "../io/video.h"
#include "../../../../shared/conversions.h"

namespace modules {
    static uint8_t* structure_address;

    void discover(uint8_t* structure_addr) {
        structure_address = structure_addr;

        uint32_t mod_count = *(uint32_t*)(structure_address + 20);
        uint32_t* mod_addr_first = reinterpret_cast<uint32_t*>(*(uint32_t*)(structure_address + 24));

        for (int i = 0; i < mod_count; i++) {
            modules::MultibootModule* mod = reinterpret_cast<modules::MultibootModule*>(mod_addr_first + (i * 4));

            video::printf("Module ");
            video::printf(conversions::s_int_to_char(i + 1));
            video::printf(" name: ");
            video::printf(mod->string);
            video::printnl();
            video::printnl();
        }
    }
};