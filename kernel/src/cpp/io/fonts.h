#pragma once

#include "stdint.h"
#include "../misc/smart_ptr.h"

namespace font_interpreter {
    // Resizes the specified character using the nearest neighbour algorithm and returns the resulting pixels in a smart pointer.
    smart_ptr<uint16_t> resize_char_nn(uint16_t* font, uint8_t target_width, uint8_t target_height);

    uint16_t* load_char(uint16_t* font, uint8_t character);

    // Returns the width of the specified character.
    uint8_t char_width(uint16_t* font, uint8_t character);

    // Returns the height of the specified character.
    uint8_t char_height(uint16_t* font, uint8_t character);
}