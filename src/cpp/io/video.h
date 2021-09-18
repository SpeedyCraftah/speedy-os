#pragma once

#include <stdint.h>

#include "vga_colours.h"
#include "../structures/string.h"

class video {
    public:
        static void clearscr(const VGA_COLOUR bg = VGA_COLOUR::BLACK);
        static void printnl();
        static void printf(
            char* input,
            const VGA_COLOUR text_colour = VGA_COLOUR::LIGHT_GREY,
            const VGA_COLOUR bg_colour = (VGA_COLOUR)default_background
        );
        static void printf(
            char input,
            const VGA_COLOUR text_colour = VGA_COLOUR::LIGHT_GREY,
            const VGA_COLOUR bg_colour = (VGA_COLOUR)default_background
        );

    private:
        static unsigned int VGA_WIDTH;
        static unsigned int VGA_HEIGHT;
        static unsigned int VGA_NULL;

        static unsigned short* current_address;
        static unsigned short* address;

        static unsigned short default_background;

        static uint16_t add_colour_to_char(
            const char c,
            const VGA_COLOUR text = VGA_COLOUR::WHITE,
            const VGA_COLOUR bg = (VGA_COLOUR)default_background
        );
};