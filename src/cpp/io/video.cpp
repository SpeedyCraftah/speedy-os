#include "video.h"

// Video dimensions and constants.
unsigned int video::VGA_HEIGHT = 25;
unsigned int video::VGA_WIDTH = 80;

// Video memory address.
unsigned short* video::address = (unsigned short*)0xb8000;
unsigned short* video::current_address = (unsigned short*)0xb8000;

// Current background.
unsigned short video::default_background = VGA_COLOUR::BLACK;

inline uint16_t video::add_colour_to_char(const char c, const VGA_COLOUR text, const VGA_COLOUR bg) {
    return (uint16_t)c | (text | bg << 4) << 8;
}

void video::savescr() {
    saved_current_address = current_address;
    
    for (uint32_t i = 0; i < 80 * 25; i++) {
        saved_screen_state[i] = address[i];
    }
}

void video::restorescr() {
    current_address = saved_current_address;
    
    for (uint32_t i = 0; i < 80 * 25; i++) {
        address[i] = saved_screen_state[i];
    }
}

void video::clearscr(const VGA_COLOUR bg) {
    for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        address[i] = add_colour_to_char(' ', bg, bg);
    }
    
    default_background = bg;
    current_address = address;
}

void video::printnl() {
    int used_addresses = current_address - address;
    int addresses_to_skip = VGA_WIDTH - (used_addresses % VGA_WIDTH);

    current_address = current_address + (addresses_to_skip == 0 ? VGA_WIDTH : addresses_to_skip);
}

// Prints following char stream to display with optional colours.
void video::printf(char* input, const VGA_COLOUR text_colour, const VGA_COLOUR bg_colour) {
    int i = 0;

    while (1) {
        if (input[i] == '\0') {
            break;
        } else if (input[i] == '\n') {
            printnl();
        } else {
            current_address[0] = add_colour_to_char(input[i], text_colour, bg_colour);
            current_address++;
        }

        i++;
    }
}

// Print char.
void video::printf(char input, const VGA_COLOUR text_colour, const VGA_COLOUR bg_colour) {
    current_address[0] = add_colour_to_char(input, text_colour, bg_colour);
    current_address++;
}

// Prints to the screen but in reverse.
void video::printf_reverse(char* input, const VGA_COLOUR text_colour, const VGA_COLOUR bg_colour) {
    int i = 0;

    while (1) {
        if (input[i] == '\0') {
            break;
        } else if (input[i] == '\n') {
            printnl();
        } else {
            current_address[0] = add_colour_to_char(input[i], text_colour, bg_colour);
            current_address--;
        }

        i++;
    }

    current_address++;
}

void video::printf_log(char* name, char* input, const VGA_COLOUR name_colour, const VGA_COLOUR input_colour) {
    video::printf("[", VGA_COLOUR::WHITE);
    video::printf(name, name_colour);
    video::printf("] ", VGA_COLOUR::WHITE);
    video::printf(input, input_colour);
    video::printnl();
}
