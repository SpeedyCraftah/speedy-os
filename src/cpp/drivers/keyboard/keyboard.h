#pragma once

#include "../../structures/string.h"
#include <stdint.h>

enum MODIFIER_KEYS {
	ENTER_PRESSED = 0x1C,
	ENTER_RELEASED = 0x9C,

	ESCAPE_PRESSED = 0x01,
	ESCAPE_RELEASED = 0x81,

	BACKSPACE_PRESSED = 0x0E,
	BACKSPACE_RELEASED = 0x8E,

	CAPSLOCK_PRESSED = 0x3A,
	CAPSLOCK_RELEASED = 0xBA,

	LEFTSHIFT_PRESSED = 0x2A,
	LEFTSHIFT_RELEASED = 0xAA,

	LEFTCTRL_PRESSED = 0x1D,
	LEFTCTRL_RELEASED = 0x9D
};

namespace drivers {
    class keyboard {
        public:
			static char keys_pressed_map[256];
			static char keys_released_map[256];

            static void load();

			static void handle_interrupt();

			// Get some extra performance by passing params via registers.
            static __attribute__((fastcall)) char keycode_to_ascii(uint8_t keycode, bool pressed);
			static __attribute__((fastcall)) void ascii_to_uppercase(structures::string& text);
			static __attribute__((fastcall)) char ascii_to_uppercase(char character);

			static void setup_char_table();
    };
}