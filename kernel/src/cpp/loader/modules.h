#pragma once

#include "stdint.h"

// Handles the Grub modules and converts it to a list of loadable programs.

namespace modules {
    struct MultibootModule {
        uint8_t* mod_start;
        uint8_t* mod_end;
        char* string;
        uint32_t _reserved;
    }__attribute__((packed));

    void discover(uint8_t* structure_address);
}