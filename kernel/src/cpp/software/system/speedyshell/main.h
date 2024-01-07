#pragma once

#include "stdint.h"
#include "../../include/sys.h"
#include "../../../io/graphics.h"

namespace speedyshell {
    extern uint32_t running_process_id;
    extern uint32_t input_thread_id;
    extern char text_buffer[100];
    extern uint8_t text_buffer_position;
    extern bool caps_text;
    extern bool allow_typing;
    extern bool input_mode;
    extern bool pixel_mode;

    void printf(char* text, speedyos::VGAColour colour = speedyos::VGAColour::WHITE, speedyos::VGAColour back_colour = speedyos::VGAColour::BLACK);
    void start();
    void clear_buffer();
}