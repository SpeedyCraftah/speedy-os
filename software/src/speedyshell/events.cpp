#include "main.h"

#include "../../include/sys.h"

void on_process_end(uint32_t id, uint32_t pid) {
    return speedyos::end_event();
}

void on_key_press(uint32_t id, uint32_t data) {
    if (!input_allowed) goto end_event;

    char character;
    character = data;

    // Add character to buffer if possible.
    if (text_buffer_ptr >= MAX_TEXT_BUFFER_SIZE) goto end_event;
    text_buffer[text_buffer_ptr] = character;
    text_buffer_ptr++;

    end_event:
    return speedyos::end_event();
}

void on_modifier_press(uint32_t id, uint32_t data) {
    if (data == speedyos::ModifierKeys::CAPSLOCK_PRESSED) {
        caps_text = !caps_text;
    } else if (data == speedyos::ModifierKeys::LEFTSHIFT_PRESSED) {
        caps_text = true;
    } else if (data == speedyos::ModifierKeys::LEFTSHIFT_RELEASED) {
        caps_text = false;
    }

    return speedyos::end_event();
}