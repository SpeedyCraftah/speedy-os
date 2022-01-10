#pragma once

#include <stdint.h>

#include "graphics.h"
#include "../structures/string.h"

class video {
    public:
        static uint32_t x_offset;
        static uint32_t y_offset;

        static uint32_t* address;
        static uint32_t* current_address;
        
        static unsigned int VGA_WIDTH;
        static unsigned int VGA_HEIGHT;

        struct character_record {
            uint32_t x;
            uint32_t y;
            uint32_t width;
            uint32_t height;
        };

        static structures::flexible_array<character_record> character_records;

        static void printnl();

        static void printf(
            char* input,
            const uint32_t text_colour = 0xD3D3D3,
            const uint32_t bg_colour = default_background
        );

        static void printf(
            char input,
            const uint32_t text_colour = 0xD3D3D3,
            const uint32_t bg_colour = default_background
        );

        static void printf_log(
            char* name,
            char* input,
            const uint32_t name_colour = VGA_COLOUR::LIGHT_BLUE,
            const uint32_t input_colour = VGA_COLOUR::WHITE
        );

        static void clearscr(const uint32_t bg = VGA_COLOUR::BLACK);

        static void savescr();
        static void restorescr();

        // Clears the latest character and puts the cursor there.
        static void printf_reverse(
            uint32_t distance = 1
        );

    private:
        static uint32_t saved_x_offset;
        static uint32_t saved_y_offset;

        static uint32_t saved_screen_state[800 * 600];
        static uint32_t* saved_current_address;
    
        static uint32_t default_background;
};
