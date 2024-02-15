#include "modules.h"

#include "../io/video.h"
#include "../../../../shared/conversions.h"
#include "../../../../shared/string.h"

namespace modules {
    static uint8_t* structure_address;
    structures::map<MultibootModule*>* modules;

    void init() {
        modules = new structures::map<MultibootModule*>(5);
    }

    void discover(uint8_t* structure_addr) {
        structure_address = structure_addr;

        uint32_t mod_count = *(uint32_t*)(structure_address + 20);
        uint32_t* mod_addr_first = reinterpret_cast<uint32_t*>(*(uint32_t*)(structure_address + 24));

        for (int i = 0; i < mod_count; i++) {
            MultibootModule* module = new MultibootModule;
            RawMultibootModule* mod = reinterpret_cast<RawMultibootModule*>(mod_addr_first + (i * 4));
            auto args = structures::string(mod->string).split_by(' ', false);

            // NOTE - this is on the heap.
            char* mod_name = args[0];

            module->name = mod_name;
            module->type = MultibootModule::SOFTWARE_EXECUTABLE;
            module->size = reinterpret_cast<uint32_t>(mod->mod_end) - reinterpret_cast<uint32_t>(mod->mod_start);
            module->data = mod->mod_start;

            video::printf_log("Module Discovery", module->name);

            for (int j = 1; j < args.get_size(); j++) {
                auto arg_raw = args[j];
                structures::string arg = arg_raw;
                
                if (arg == "interface-provider") {
                    module->type = MultibootModule::SOFTWARE_EXECUTABLE_INTERFACE_PROVIDER;
                }

                delete arg_raw;               
            }

            video::printf_log("Module Type", conversions::u_int_to_char(module->type));

            modules->set(module->name, module);
        }
    }

    MultibootModule* find_interface_provider() {
        auto iterator = modules->create_iterator();
        while (iterator.hasNext()) {
            MultibootModule* module = iterator.next();
            if (module->type == MultibootModule::SOFTWARE_EXECUTABLE_INTERFACE_PROVIDER) {
                return module;
            }
        }

        return nullptr;
    }
};