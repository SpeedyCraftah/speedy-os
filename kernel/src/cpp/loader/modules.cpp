#include "modules.h"

#include "../io/video.h"
#include "../../../../shared/conversions.h"
#include "../../../../shared/string.h"

namespace modules {
    static uint8_t* structure_address;

    void discover(uint8_t* structure_addr) {
        structure_address = structure_addr;

        uint32_t mod_count = *(uint32_t*)(structure_address + 20);
        uint32_t* mod_addr_first = reinterpret_cast<uint32_t*>(*(uint32_t*)(structure_address + 24));

        for (int i = 0; i < mod_count; i++) {
            modules::MultibootModule* mod = reinterpret_cast<modules::MultibootModule*>(mod_addr_first + (i * 4));
            auto args = structures::string(mod->string).split_by(' ');
            char* mod_name = args[0];

            video::printf("Discovered module: ");
            video::printf(mod_name);
            video::printnl();

            for (int j = 1; j < args.get_size(); j++) {
                auto arg = args[j];



                delete arg;               
            }
        }
    }
};