#pragma once

#include <stdint.h>

#include "graphics.h"
#include "../structures/string.h"

// Basic video and printing functions for kernel.
namespace video {
    extern uint32_t default_background;

    void printnl();
    void printf(char* input, const uint32_t text_colour = 0xD3D3D3, const uint32_t bg_colour = default_background);
    void printf(char input, const uint32_t text_colour = 0xD3D3D3, const uint32_t bg_colour = default_background);
    void printf_log(char* name, char* input, const uint32_t name_colour = VGA_COLOUR::LIGHT_BLUE, const uint32_t input_colour = VGA_COLOUR::WHITE);
    void clearscr(const uint32_t bg = VGA_COLOUR::BLACK);
};