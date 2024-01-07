#include "video.h"

#include "fonts.h"
#include "fonts/internal.h"
#include "graphics.h"

// Video dimensions and constants.
unsigned int video::VGA_HEIGHT = 600;
unsigned int video::VGA_WIDTH = 800;

uint32_t video::x_offset = 0;
uint32_t video::y_offset = 0;

uint32_t video::saved_x_offset = 0;
uint32_t video::saved_y_offset = 0;

//structures::flexible_array<video::character_record> video::character_records
//    = structures::flexible_array<video::character_record>(80 * 100);

// Video memory address.
uint32_t* video::address = (uint32_t*)0xA0000;
uint32_t* video::current_address = (uint32_t*)0xA0000;

// Video saving.
uint32_t video::saved_screen_state[800 * 600];
uint32_t* video::saved_current_address = video::address;

// Current background.
uint32_t video::default_background = 0x000000;

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

void video::printnl() {
    y_offset += 15;
    x_offset = 0;
}

// Prints following char stream to display with optional colours.
void video::printf(char* input, const uint32_t text_colour, const uint32_t bg_colour) {
    int i = 0;

    while (1) {
        if (input[i] == '\0') {
            break;
        } else if (input[i] == '\n') {
            printnl();
        } else {
            // Load character.
            uint16_t* data = font_interpreter::load_char(
                internal_fonts::bios_port_improved, input[i]
            );

            uint16_t width = data[0];
            uint16_t height = data[1];

            // Save character.
            /*character_record record;
            record.x = x_offset;
            record.y = y_offset;
            record.width = width;
            record.height = height;

            character_records.push(record);*/

            data = data + 2;

            if (width + x_offset > VGA_WIDTH) {
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

        i++;
    }
}

// Prints a single character.
void video::printf(char input, const uint32_t text_colour, const uint32_t bg_colour) {
    // Load character.
    uint16_t* data = font_interpreter::load_char(
        internal_fonts::bios_port_improved, input
    );

    uint16_t width = data[0];
    uint16_t height = data[1];

    // Save character.
    /*character_record record;
    record.x = x_offset;
    record.y = y_offset;
    record.width = width;
    record.height = height;

    character_records.push(record);*/

    data = data + 2;

    if (width + x_offset > VGA_WIDTH) {
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

void video::printf_log(char* name, char* input, const uint32_t name_colour, const uint32_t input_colour) {
    video::printf("[", 0xFFFFFF);
    video::printf(name, name_colour);
    video::printf("] ", 0xFFFFFF);
    video::printf(input, input_colour);
    video::printnl();
}

void video::clearscr(const uint32_t bg) {
    video::default_background = bg;

    graphics::fill_colour = bg;
    graphics::outline_colour = bg;

    graphics::draw_rectangle(0, 0, VGA_WIDTH, VGA_HEIGHT, true);

    video::x_offset = 0;
    video::y_offset = 0;

    //video::character_records.reset();
}

void video::savescr() {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video::saved_screen_state[i] = graphics::double_buffer[i];
    }

    video::saved_x_offset = video::x_offset;
    video::saved_y_offset = video::y_offset;
}

void video::restorescr() {
    video::x_offset = video::saved_x_offset;
    video::y_offset = video::saved_y_offset;

    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        //graphics::video_address[i] = video::saved_screen_state[i];
        //graphics::double_buffer[i] = video::saved_screen_state[i];
        graphics::draw_pixel_linear(i, video::saved_screen_state[i]);
    }
}

void video::printf_reverse(uint32_t distance) {
    /*if (character_records.get_size() < distance) 
        distance = character_records.get_size();

    for (uint32_t i = 0; i < distance; i++) {
        // Get last character.
        character_record record = character_records.pop();
        x_offset = record.x;
        y_offset = record.y;

        graphics::fill_colour = video::default_background;
        graphics::outline_colour = video::default_background;

        // Draw over the character.
        graphics::draw_rectangle(
            x_offset, y_offset,
            record.width + 1, record.height + 1,
            true
        );
    }*/
}