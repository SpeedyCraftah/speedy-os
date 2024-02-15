#pragma once

#include "stdint.h"
#include "../../../../shared/map.h"

// Handles the Grub modules and converts it to a list of loadable programs.

namespace modules {
    struct RawMultibootModule {
        uint8_t* mod_start;
        uint8_t* mod_end;
        char* string;
        uint32_t _reserved;
    }__attribute__((packed));

    struct MultibootModule {
        char* name;
        enum : uint8_t {
            SOFTWARE_EXECUTABLE,
            SOFTWARE_EXECUTABLE_INTERFACE_PROVIDER
        } type;

        uint32_t size;
        uint8_t* data;
    };

    extern structures::map<MultibootModule*>* modules;

    void init();
    void discover(uint8_t* structure_address);

    MultibootModule* find_interface_provider();
}