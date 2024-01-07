#pragma once

#include "stdint.h"

namespace vga_registers {
   void set_value(uint16_t port, uint8_t index, uint8_t value);
   uint8_t read_value(uint16_t port, uint8_t index);
}
