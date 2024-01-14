#include "video.h"

#include "fonts.h"
#include "fonts/internal.h"
#include "graphics.h"

// For C.
extern "C" void printnl() {
    video::printnl();
}
extern "C" void printf(char* input, uint32_t text_colour, uint32_t bg_colour) {
    video::printf(input, text_colour, bg_colour);
}
extern "C" void printf_log(char* name, char* input, const uint32_t name_colour, const uint32_t input_colour) {
    video::printf_log(name, input, name_colour, input_colour);
}

namespace video {
    uint32_t x_offset = 0;
    uint32_t y_offset = 0;
    uint32_t default_background = 0x00000000;

    void printnl() {
        y_offset += 15;
        x_offset = 0;
    }

    // Prints following char stream to display with optional colours.
    void printf(char* input, const uint32_t text_colour, const uint32_t bg_colour) {
        for (int i = 0; input[i] != '\0'; i++) {
            if (input[i] == '\n') printnl();
            else {
                // Load character.
                uint16_t* data = font_interpreter::load_char(
                    internal_fonts::bios_port_improved, input[i]
                );

                uint16_t width = data[0];
                uint16_t height = data[1];

                data = data + 2;

                if (width + x_offset > graphics::resolution_width) {
                    y_offset += height + 1;
                    x_offset = 0;
                }
                
                for (uint32_t y = 0; y < height; y++) {
                    uint32_t offset_y = y * width;

                    for (uint32_t x = 0; x < width; x++) {
                        if (data[offset_y + x] == 1) {
                            // Draw pixel.
                            graphics::draw_pixel(x + x_offset, y + y_offset, text_colour);
                        } else {
                            // Draw background.
                            if (default_background != bg_colour)
                                graphics::draw_pixel(x + x_offset, y + y_offset, bg_colour);
                        }
                    }
                }

                x_offset += width + 2;
            }
        }
    }

    void printf(char input, const uint32_t text_colour, const uint32_t bg_colour) {
        // Load character.
        uint16_t* data = font_interpreter::load_char(
        internal_fonts::bios_port_improved, input
        );

        uint16_t width = data[0];
        uint16_t height = data[1];

        if (width + x_offset > graphics::resolution_width) {
            y_offset += height + 1;
            x_offset = 0;
        }

        for (uint32_t y = 0; y < height; y++) {
            uint32_t offset_y = y * width;

            for (uint32_t x = 0; x < width; x++) {
                if (data[offset_y + x] == 1) {
                    // Draw pixel.
                    graphics::draw_pixel(x + x_offset, y + y_offset, text_colour);
                } else {
                    // Draw background.
                    if (default_background != bg_colour) graphics::draw_pixel(x + x_offset, y + y_offset, bg_colour);
                }
            }
        }

        x_offset += width + 2;
    }

    void printf_log(char* name, char* input, const uint32_t name_colour, const uint32_t input_colour) {
        video::printf("[", 0xFFFFFF);
        video::printf(name, name_colour);
        video::printf("] ", 0xFFFFFF);
        video::printf(input, input_colour);
        video::printnl();
    }

    void clearscr(const uint32_t bg) {
        default_background = bg;
        x_offset = 0;
        y_offset = 0;

        graphics::fill_colour = bg;
        graphics::draw_rectangle_no_border(0, 0, graphics::resolution_width, graphics::resolution_height);
    }
};