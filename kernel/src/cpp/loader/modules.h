#pragma once

#include "stdint.h"

// Handles the Grub modules and converts it to a list of loadable programs.

namespace modules {
    void discover(uint8_t* structure_address);
}