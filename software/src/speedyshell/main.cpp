#include "../../include/sys.h"
#include "../../../shared/graphics/graphics.h"
#include "../../../shared/graphics/fonts/internal.h"
#include "../../include/sys_additional.h"
#include "../../include/mutex.h"
#include "main.h"
#include "shell.h"
#include "events.h"

uint32_t output_datasink_id = 0;
bool input_allowed = false;
bool caps_text = false;
uint32_t running_pid = 0;

char text_buffer[MAX_TEXT_BUFFER_SIZE];
uint32_t text_buffer_ptr = 0;

uint32_t max_char_height = 0;
uint32_t x_offset = 0;
uint32_t y_offset = 0;

uint32_t cursor_x = 0;
uint32_t cursor_y = 0;
uint32_t cursor_thread_id = 0;

ThreadMutex cursor_mutex;
bool cursor_toggle = true;
void cursor_blink_thread(void* c) {
    while (true) {
        cursor_mutex.lock();
        graphics::fill_colour = cursor_toggle ? rgb_colour(255, 255, 255) : rgb_colour(0, 0, 0);
        graphics::draw_text(internal_fonts::bios_port_improved, cursor_x, cursor_y, "_");

        cursor_toggle = !cursor_toggle;
        cursor_mutex.unlock();
        speedyos::suspend_thread(500);
    }
}

void cursor_remove() {
    // Suspend the cursor thread.
    speedyos::park_thread(cursor_thread_id);

    graphics::fill_colour = rgb_colour(0, 0, 0);
    graphics::draw_text(internal_fonts::bios_port_improved, cursor_x, cursor_y, "_");
}

void cursor_move(uint32_t new_x, uint32_t new_y, bool remove_old) {
    if (remove_old) {
        cursor_mutex.lock();
    
        graphics::fill_colour = rgb_colour(0, 0, 0);
        graphics::draw_text(internal_fonts::bios_port_improved, cursor_x, cursor_y, "_");
    }

    cursor_toggle = true;
    cursor_x = new_x;
    cursor_y = new_y;

    if (remove_old) {
        graphics::fill_colour = rgb_colour(255, 255, 255);
        graphics::draw_text(internal_fonts::bios_port_improved, cursor_x, cursor_y, "_");
        cursor_mutex.unlock();
    }
}

void cursor_instate() {
    graphics::fill_colour = rgb_colour(255, 255, 255);
    graphics::draw_text(internal_fonts::bios_port_improved, cursor_x, cursor_y, "_");
    cursor_toggle = true;

    // Unpark the cursor thread.
    speedyos::awake_thread(cursor_thread_id);
}

void printnl() {
    if (y_offset + max_char_height + 16 >= graphics::resolution_height) {
        speedyos::park_thread(cursor_thread_id);
        graphics::shift_screen_horizontal(16);
    } else {
        y_offset += 16;
    }

    x_offset = 0;
}

void print(char* text, uint32_t colour) {
    graphics::fill_colour = colour;

    char c[2];
    c[1] = 0;
    while ((c[0] = *text++) != '\0') {
        if (c[0] == '\n') {
            printnl();
        } else {
            if (x_offset + font_interpreter::char_width(internal_fonts::bios_port_improved, c[0]) >= graphics::resolution_width) printnl();
            x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, c);
        }
    }
};

int main() {
    //speedyos::suspend_thread(4000);

    // Get datasink IDs from the kernel.
    output_datasink_id = speedyos::interface_provider::fetch_output_sink_id();

    max_char_height = font_interpreter::char_height(internal_fonts::bios_port_improved, '!');

    // Upgrade graphics mode and setup addresses.
    graphics::init(
        (uint32_t*)speedyos::upgrade_graphics(),
        speedyos::fetch_graphics_resolution(),
        speedyos::fetch_colour_depth()
    );

    // Find keyboard PID and register event handlers.
    uint32_t keyboard_pid = speedyos::fetch_process_id_by_string("Keyboard Driver");

    // Register events.
    speedyos::register_event_for_thread(keyboard_pid, 1, on_key_press);
    speedyos::register_event_for_thread(keyboard_pid, 4 | 8, on_modifier_press);

    // Hook scheduler for process events.
    uint32_t scheduler_pid = speedyos::fetch_process_id_by_string("Scheduler Events");

    speedyos::register_event_for_thread(scheduler_pid, 2, on_process_end);
    
    // Clear the screen.
    graphics::outline_colour = 0x0;
    graphics::fill_colour = 0x0;
    graphics::draw_rectangle(0, 0, graphics::resolution_width, graphics::resolution_height, true);

    graphics::outline_colour = 0xFFFFFFFF;
    graphics::fill_colour = 0xFFFFFFFF;
    
    print_prefix();
    cursor_x = x_offset;

    cursor_thread_id = speedyos::create_thread(cursor_blink_thread);

    input_allowed = true;

    speedyos::update_execution_policy(speedyos::ThreadExecutionPolicy::EVENT_ONLY);

    return 0;
}