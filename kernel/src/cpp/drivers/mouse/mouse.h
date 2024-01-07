#pragma once

#include "stdint.h"

enum MOUSE_BUTTONS {
    LEFT_PRESS = 0,
    LEFT_RELEASE = 1,

    RIGHT_PRESS = 2,
    RIGHT_RELEASE = 3,

    MIDDLE_PRESS = 4,
    MIDDLE_RELEASE = 5,
};

namespace drivers {
    class mouse {
        public:
            // Keep track of pressed buttons to avoid multiple event emits.
            static bool right_button_pressed;
            static bool left_button_pressed;
            static bool middle_button_pressed;

            static void load();
            static void handle_interrupt();

            static void write(uint8_t data);
            static void wait(bool type);
            static uint8_t read();
    };
}
