#include "../../include/sys.h"
#include "../../../shared/graphics/graphics.h"

#include "events.h"

bool input_allowed = false;
bool caps_text = false;
uint32_t running_pid = 0;

char text_buffer[100];
uint32_t text_buffer_ptr = 0;

int main() {
    speedyos::suspend_thread(4000);

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
    graphics::draw_rectangle(0, 0, graphics::resolution_width, graphics::resolution_height);

    return 0;
}