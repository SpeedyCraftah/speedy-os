#include "keyboard.h"

#include "../../interrupts/irq.h"
#include "../../io/video.h"
#include "../../abstractions/io_port.h"
#include "../../chips/pic.h"
#include "../../scheduling/scheduler.h"
#include "../../scheduling/events.h"
#include "../../misc/random.h"

// Keyboard driver events.
// ID 1 - On key press.
// ID 2 - On key release.
// ID 4 - On modifier key press.
// ID 8 - On modifier key release.

namespace drivers {
    static Process* process;

    void keyboard::load() {
        setup_char_table();

        ProcessFlags flags;
        flags.system_process = true;
        flags.virtual_process = true;

        // Create virtual process for events to take place.
        process = scheduler::create_process(
            "Keyboard Driver",
            0,
            flags
        );
    }
    
    // Inlined for performance.
    void keyboard::handle_interrupt() {
        uint32_t key_raw = io_port::bit_8::in(0x60);

        // If the key requires more than one data packet (only supports two max at the moment).
        if (key_raw == 0xE0) {
            key_raw = 0xFF + io_port::bit_8::in(0x60);
        }

        // Key presses.
        char char_press = keyboard::keycode_to_ascii(key_raw, true);
        if (char_press != 0) {
            scheduler::events::emit_event(process, 1, char_press);
            return;
        }

        // Key releases.
        char char_release = keyboard::keycode_to_ascii(key_raw, false);
        if (char_release != 0) {
            scheduler::events::emit_event(process, 2, char_release);
            return;
        }

        // Modifier press.
        bool modifier_pressed = keyboard::modifier_supported(key_raw, true);
        if (modifier_pressed) {
            scheduler::events::emit_event(process, 4, key_raw);
            return;
        }

        // Modifier released.
        bool modifier_released = keyboard::modifier_supported(key_raw, false);
        if (modifier_released) {
            scheduler::events::emit_event(process, 8, key_raw);
            return;
        }
    }

    void __attribute__((fastcall)) keyboard::ascii_to_uppercase(structures::string& string) {
        for (int i = 0; i <= string.length(); i++) { 
            if (string[i] >= 97 && string[i] <= 122) {
                string[i] = string[i] - 32;
            }
        }
    }

    inline bool __attribute__((fastcall)) keyboard::modifier_supported(uint16_t key, bool pressed) {
        return pressed ? modifiers_pressed_map[key] != 0 : modifiers_released_map[key] != 0;
    }

    char __attribute__((fastcall)) keyboard::ascii_to_uppercase(char character) {
        if (character >= 97 && character <= 122) {
            return character - 32;
        } else return character;
    }

    inline char __attribute__((fastcall)) keyboard::keycode_to_ascii(uint16_t keycode, bool pressed) {
        return pressed ? keys_pressed_map[keycode] : keys_released_map[keycode];
    }

    char keyboard::keys_pressed_map[512];
    char keyboard::keys_released_map[512];
    char keyboard::modifiers_pressed_map[512];
    char keyboard::modifiers_released_map[512];

    void keyboard::setup_char_table() {
        // Scan set 1 pressed normal characters.

        keys_pressed_map[0x04] = '3';
        keys_pressed_map[0x08] = '7';
        keys_pressed_map[0x10] = 'q';
        keys_pressed_map[0x14] = 't';
        keys_pressed_map[0x18] = 'o';
        keys_pressed_map[0x20] = 'd';
        keys_pressed_map[0x24] = 'j';
        keys_pressed_map[0x2C] = 'z';
        keys_pressed_map[0x30] = 'b';
        keys_pressed_map[0x05] = '4';
        keys_pressed_map[0x09] = '8';
        keys_pressed_map[0x11] = 'w';
        keys_pressed_map[0x15] = 'y';
        keys_pressed_map[0x19] = 'p';
        keys_pressed_map[0x21] = 'f';
        keys_pressed_map[0x25] = 'k';
        keys_pressed_map[0x2D] = 'x';
        keys_pressed_map[0x31] = 'n';
        keys_pressed_map[0x39] = ' ';
        keys_pressed_map[0x02] = '1';
        keys_pressed_map[0x06] = '5';
        keys_pressed_map[0x0A] = '9';
        keys_pressed_map[0x12] = 'e';
        keys_pressed_map[0x16] = 'u';
        keys_pressed_map[0x1E] = 'a';
        keys_pressed_map[0x22] = 'g';
        keys_pressed_map[0x26] = 'l';
        keys_pressed_map[0x2E] = 'c';
        keys_pressed_map[0x32] = 'm';
        keys_pressed_map[0x03] = '2';
        keys_pressed_map[0x07] = '6';
        keys_pressed_map[0x0B] = '0';
        keys_pressed_map[0x13] = 'r';
        keys_pressed_map[0x17] = 'i';
        keys_pressed_map[0x1F] = 's';
        keys_pressed_map[0x23] = 'h';
        keys_pressed_map[0x2F] = 'v';

        // Released keys.
        keys_released_map[0x84] = '3';
        keys_released_map[0x88] = '7';
        keys_released_map[0x90] = 'q';
        keys_released_map[0x94] = 't';
        keys_released_map[0x98] = 'o';
        keys_released_map[0xA0] = 'd';
        keys_released_map[0xA4] = 'j';
        keys_released_map[0xAC] = 'z';
        keys_released_map[0xB0] = 'b';
        keys_released_map[0x85] = '4';
        keys_released_map[0x89] = '8';
        keys_released_map[0x91] = 'w';
        keys_released_map[0x95] = 'y';
        keys_released_map[0x99] = 'p';
        keys_released_map[0xA1] = 'f';
        keys_released_map[0xA5] = 'k';
        keys_released_map[0xAD] = 'x';
        keys_released_map[0xB1] = 'n';
        keys_released_map[0xB9] = ' ';
        keys_released_map[0x82] = '1';
        keys_released_map[0x86] = '5';
        keys_released_map[0x8A] = '9';
        keys_released_map[0x92] = 'e';
        keys_released_map[0x96] = 'u';
        keys_released_map[0x9E] = 'a';
        keys_released_map[0xA2] = 'g';
        keys_released_map[0xA6] = 'l';
        keys_released_map[0xAE] = 'c';
        keys_released_map[0xB2] = 'm';
        keys_released_map[0x83] = '2';
        keys_released_map[0x87] = '6';
        keys_released_map[0x8B] = '0';
        keys_released_map[0x93] = 'r';
        keys_released_map[0x97] = 'i';
        keys_released_map[0x9F] = 's';
        keys_released_map[0xA3] = 'h';
        keys_released_map[0xAF] = 'v';

        // Special characters pressed.
        keys_pressed_map[0x0C] = '-';
        keys_pressed_map[0x28] = '\'';
        keys_pressed_map[0x34] = '.';
        keys_pressed_map[0x0D] = '=';
        keys_pressed_map[0x29] = '`';
        keys_pressed_map[0x35] = '/';
        keys_pressed_map[0x1A] = '[';
        keys_pressed_map[0x1B] = ']';
        keys_pressed_map[0x27] = ';';
        keys_pressed_map[0x2B] = '\\';
        keys_pressed_map[0x33] = ',';

        // Special characters released.
        keys_released_map[0x8C] = '-';
        keys_released_map[0xA8] = '\'';
        keys_released_map[0xB4] = '.';
        keys_released_map[0x8D] = '=';
        keys_released_map[0xA9] = '`';
        keys_released_map[0xB5] = '/';
        keys_released_map[0x9A] = '[';
        keys_released_map[0x9B] = ']';
        keys_released_map[0xA7] = ';';
        keys_released_map[0xAB] = '\\';
        keys_released_map[0xB3] = ',';

        // Modifier keys.
        // Set up will be improved eventually.
        // More keys will also be added soon.

        // Modifier pressed.
        modifiers_pressed_map[0x1C] = 1;
        modifiers_pressed_map[0x01] = 1;
        modifiers_pressed_map[0x0E] = 1;
        modifiers_pressed_map[0x3A] = 1;
        modifiers_pressed_map[0x2A] = 1;
        modifiers_pressed_map[0x1D] = 1;
        modifiers_pressed_map[0xFF + 0x48] = 1;
        modifiers_pressed_map[0xFF + 0x50] = 1;
        modifiers_pressed_map[0xFF + 0x4B] = 1;
        modifiers_pressed_map[0xFF + 0x4D] = 1;

        // Modifier released.
        // Contains code of pressed keys for filtering of
        // pressed down keys.
        modifiers_released_map[0x9C] = 0x1C;
        modifiers_released_map[0x81] = 0x01;
        modifiers_released_map[0x8E] = 0x0E;
        modifiers_released_map[0xBA] = 0x3A;
        modifiers_released_map[0xAA] = 0x2A;
        modifiers_released_map[0x9D] = 0x1D;
        modifiers_pressed_map[0xFF + 0xC8] = 1;
        modifiers_pressed_map[0xFF + 0xD0] = 1;
        modifiers_pressed_map[0xFF + 0xCB] = 1;
        modifiers_pressed_map[0xFF + 0xCD] = 1;
    }
}