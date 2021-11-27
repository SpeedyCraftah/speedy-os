#pragma once

#include "../../../io/video.h"

namespace speedyshell {
    extern uint32_t process_id;
    extern uint32_t running_process_id;

    extern char terminal_buffer[25][80];
    extern char text_input[80];
    extern uint32_t text_input_position;

    void start();
    void printf(char* text, VGA_COLOUR colour = VGA_COLOUR::WHITE, VGA_COLOUR back_colour = VGA_COLOUR::BLACK);
    uint32_t get_text_input_position();
    char* get_text_input();

    void print_prefix();
    void clear_input();
}