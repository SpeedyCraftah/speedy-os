#include "modules.h"

#include "../io/video.h"
#include "../../../../shared/conversions.h"
#include "../../../../shared/string.h"

namespace modules {
    static uint8_t* structure_address;
    uint32_t modules_count;
    MultibootModule* modules;

    void discover(uint8_t* structure_addr) {
        structure_address = structure_addr;

        uint32_t mod_count = *(uint32_t*)(structure_address + 20);
        uint32_t* mod_addr_first = reinterpret_cast<uint32_t*>(*(uint32_t*)(structure_address + 24));

        modules_count = mod_count;
        modules = (MultibootModule*)kmalloc(sizeof(MultibootModule) * mod_count);

        for (int i = 0; i < mod_count; i++) {
            MultibootModule& module = modules[i];
            RawMultibootModule* mod = reinterpret_cast<RawMultibootModule*>(mod_addr_first + (i * 4));
            auto args = structures::string(mod->string).split_by(' ');

            // NOTE - this is on the heap.
            char* mod_name = args[0];

            module.name = mod_name;
            module.type = MultibootModule::SOFTWARE_EXECUTABLE;
            module.size = reinterpret_cast<uint32_t>(mod->mod_end) - reinterpret_cast<uint32_t>(mod->mod_start);
            module.data = mod->mod_start;

            video::printf("Discovered module: ");
            video::printf(mod_name);

            for (int j = 1; j < args.get_size(); j++) {
                auto arg_raw = args[j];
                structures::string arg = arg_raw;
                
                if (arg == "interface-provider") {
                    module.type = MultibootModule::SOFTWARE_EXECUTABLE_INTERFACE_PROVIDER;
                }

                delete arg_raw;               
            }

            video::printf(", type ");
            video::printf(conversions::u_int_to_char(module.type));
            video::printnl();
        }

        video::printnl();
    }
};