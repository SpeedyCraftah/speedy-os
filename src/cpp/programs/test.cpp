#include "include/sys.h"
#include "include/base_program.h"

#include "../io/video.h"
#include "../scheduling/scheduler.h"
#include "../scheduling/events.h"

namespace TestProgramCPP {
    void __attribute__((fastcall)) on_keyboard_data(uint32_t id, uint32_t data) {
        if (data == '/') {
            video::printf("\n\n[TestKeyboard] Sending end request to kernel. Bye!", VGA_COLOUR::LIGHT_CYAN);
            speedyos::end_process();
        }

        video::printf(data, VGA_COLOUR::WHITE);

        speedyos::end_event();
    }

    void main() {
        uint32_t pid = speedyos::fetch_process_id();

        video::printf("\n[TestKeyboard] Hooking into keyboard driver...\n", VGA_COLOUR::LIGHT_CYAN);

        uint32_t keyboard_driver_id = speedyos::fetch_process_id_by_string("Keyboard Driver");

        speedyos::register_event_for_process(keyboard_driver_id, 1, on_keyboard_data);

        speedyos::suspend_process(1200, speedyos::SuspensionType::FULL);
        
        video::printf("[TestKeyboard] Successfully hooked!\n", VGA_COLOUR::LIGHT_CYAN);

        video::printnl();

        video::printf("[TestKeyboard] Type something! (press / to end process)\n", VGA_COLOUR::LIGHT_CYAN);
        video::printf("[TestKeyboard] Switching into event-only mode...\n\n", VGA_COLOUR::LIGHT_CYAN);

        speedyos::update_status(speedyos::TaskStatus::RUNNING_WAITING_FOR_DATA);
    }
};