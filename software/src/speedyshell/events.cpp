#include "main.h"

#include "../../include/sys.h"
#include "../../../shared/graphics/graphics.h"
#include "../../../shared/graphics/fonts/internal.h"
#include "shell.h"

void on_process_end(uint32_t id, uint32_t pid) {
    return speedyos::end_event();
}

void on_key_press(uint32_t id, uint32_t data) {
    if (!input_allowed) return speedyos::end_event();

    char character;
    character = data;

    // Add character to buffer if possible.
    if (text_buffer_ptr >= MAX_TEXT_BUFFER_SIZE) return speedyos::end_event();
    text_buffer[text_buffer_ptr] = character;
    text_buffer_ptr++;

    char str[2] = {0, 0};
    str[0] = character;

    cursor_move(x_offset + graphics::compute_text_width(internal_fonts::bios_port_improved, str), y_offset);
    graphics::fill_colour = rgb_colour(192,192,192);
    x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, str);

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
    
    else if (data == speedyos::ModifierKeys::BACKSPACE_PRESSED) {
        if (!input_allowed) return speedyos::end_event();
        if (text_buffer_ptr == 0) return speedyos::end_event();

        char str[2] = {0, 0};
        str[0] = text_buffer[--text_buffer_ptr];

        x_offset -= graphics::compute_text_width(internal_fonts::bios_port_improved, str);
        graphics::fill_colour = 0;
        graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, str);

        cursor_move(x_offset, y_offset);
    }

    else if (data == speedyos::ModifierKeys::ENTER_PRESSED) {
        if (!input_allowed) return speedyos::end_event();
        
        cursor_remove();
        
        if (y_offset + max_char_height + 16 >= graphics::resolution_height) {
            speedyos::park_thread(cursor_thread_id);
            graphics::shift_screen_horizontal(16);
        } else {
            y_offset += 16;
        }

        x_offset = 0;

        print_prefix();
        cursor_move(x_offset, y_offset, false);
        speedyos::awake_thread(cursor_thread_id);

        text_buffer_ptr = 0;
    }



    return speedyos::end_event();
}