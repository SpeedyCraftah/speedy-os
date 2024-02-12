#include "../../include/sys.h"

bool caps_text = false;
uint32_t running_pid = 0;
char text_buffer[100];

void on_process_end(uint32_t id, uint32_t pid) {
    return speedyos::end_event();
}

void on_key_press(uint32_t id, uint32_t data) {
    return speedyos::end_event();
}

void on_modifier_press(uint32_t id, uint32_t data) {
    return speedyos::end_event();
}

int main() {
    speedyos::suspend_thread(4000);

    // Find keyboard PID and register event handlers.
    uint32_t keyboard_pid = speedyos::fetch_process_id_by_string("Keyboard Driver");

    // Register events.
    speedyos::register_event_for_thread(keyboard_pid, 1, on_key_press);
    speedyos::register_event_for_thread(keyboard_pid, 4 | 8, on_modifier_press);

    // Hook scheduler for process events.
    uint32_t scheduler_pid = speedyos::fetch_process_id_by_string("Scheduler Events");

    speedyos::register_event_for_thread(scheduler_pid, 2, on_process_end);

    return 0;
}