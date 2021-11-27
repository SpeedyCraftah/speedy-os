#include "./main.h"

#include "../../include/sys.h"
#include "../../../io/video.h"
#include "../../../structures/string.h"
#include "../../../misc/conversions.h"
#include "../../../drivers/keyboard/keyboard.h"
#include "../../../panic/panic.h"

#include "commands.h"
#include "events.h"

using namespace structures;

uint32_t speedyshell::running_process_id = 0;
uint32_t speedyshell::process_id = 0;

char speedyshell::terminal_buffer[25][80];
char speedyshell::text_input[80];
uint32_t speedyshell::text_input_position = 0;

namespace speedyshell {
    static bool caps_lock = false;
    static bool shift = false;

    uint32_t get_text_input_position() {
        return text_input_position;
    }

    char* get_text_input() {
        return text_input;
    }

    void print_prefix() {
        video::printf("<SpeedyOS@User>", VGA_COLOUR::LIGHT_GREEN);
        video::printf(" $ ", VGA_COLOUR::LIGHT_BLUE);

        // Print cursor.
        video::printf(" ", VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);
    }

    void clear_input() {
        for (uint32_t i = 0; i < text_input_position + 1; i++) {
            text_input[i] = 0;
        }

        text_input_position = 0;
    }

    bool shift_video_if_required() {
        if ((video::current_address - video::address) / video::VGA_WIDTH + 1 >= video::VGA_HEIGHT) {
            // Clear first line.
            for (uint32_t i = 0; i < 80; i++) {
                *(video::address + i) = 0;
            }

            for (uint32_t j = 0; j <= video::VGA_HEIGHT - 1; j++) {
                for (uint32_t i = 0; i < video::VGA_WIDTH; i++) {
                    auto addr = video::address + (j * 80) + i;
                    auto next_addr = video::address + ((j + 1) * 80) + i;
                    *(addr) = *next_addr;
                }
            }

            video::current_address = video::address + (video::VGA_WIDTH * (video::VGA_HEIGHT - 1));

            return true;
        }

        return false;
    }

    void printf(char* text, VGA_COLOUR colour, VGA_COLOUR back_colour) {
        uint32_t i = 0;

        while (true) {
            char c = text[i];

            if (c == '\0') {
                break;
            } else if (c == '\n') {
                video::printnl();
                shift_video_if_required();
                i++;

                continue;
            } else {
                video::printf(c, colour, back_colour);
            }

            i++;
        }

        video::printnl();
        shift_video_if_required();
    }

    void __attribute__((fastcall)) on_modifier_event(uint32_t id, uint32_t data) {
        if (running_process_id != 0) return speedyos::end_event();

        if (data == speedyos::ModifierKeys::CAPSLOCK_PRESSED) caps_lock = !caps_lock;
        else if (data == speedyos::ModifierKeys::BACKSPACE_PRESSED) {
            uint32_t remaining_spaces = video::VGA_WIDTH - ((video::current_address - video::address) % video::VGA_WIDTH);

            if (remaining_spaces >= 61) speedyos::end_event();
            else if (remaining_spaces == 0) {
                video::printf_reverse("  ");
            }
            else {
                video::printf_reverse("   ");
            }

            text_input[text_input_position - 1] = 0;
            text_input_position--;

            video::printf(" ", VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);
        } else if (data == speedyos::ModifierKeys::LEFTSHIFT_PRESSED) {
            shift = true;
        } else if (data == speedyos::ModifierKeys::LEFTSHIFT_RELEASED) {
            shift = false;
        } else if (data == speedyos::ModifierKeys::ENTER_PRESSED) {
            uint32_t remaining_spaces = video::VGA_WIDTH - ((video::current_address - video::address) % video::VGA_WIDTH);
            if (remaining_spaces != 1) video::printf_reverse("  ");

            bool r = shift_video_if_required();
            if (!r) video::printnl();

            handle_command();

            clear_input();
            print_prefix();
        } else if (data == speedyos::ModifierKeys::ESCAPE_PRESSED) {
            text_input[0] = 'i';
            text_input[1] = 'n';
            text_input[2] = 'f';
            text_input[3] = 'o';

            text_input_position = 4;

            return on_modifier_event(1, speedyos::ModifierKeys::ENTER_PRESSED);
        }

        return speedyos::end_event();
    }

    void __attribute__((fastcall)) on_key_press(uint32_t id, uint32_t data) {
        if (running_process_id != 0) return speedyos::end_event();

        uint32_t remaining_spaces = video::VGA_WIDTH - ((video::current_address - video::address) % video::VGA_WIDTH);

        if (remaining_spaces == 0) speedyos::end_event();

        // Overwrite cursor.
        video::current_address--;

        char c;

        if ((caps_lock && !shift) || (!caps_lock && shift)) c = drivers::keyboard::ascii_to_uppercase(data);
        else c = data;

        video::printf(c, VGA_COLOUR::WHITE);

        text_input[text_input_position] = c;
        text_input_position++;

        // Print new cursor.
        if (remaining_spaces != 1) video::printf(" ", VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);

        return speedyos::end_event();
    }

    void start() {
        process_id = speedyos::fetch_process_id();

        //speedyos::suspend_process(1392, speedyos::SuspensionType::FULL);
        uint32_t pid = speedyos::fetch_process_id();
        video::printf_log("SpeedyShell", string("Instance started at PID ") + conversions::s_int_to_char(pid));

        //speedyos::suspend_process(1072, speedyos::SuspensionType::FULL);
        uint32_t keyboard_pid = speedyos::fetch_process_id_by_string("Keyboard Driver");
        video::printf_log("SpeedyShell", string("Found keyboard driver at PID ") + conversions::s_int_to_char(keyboard_pid));

        //speedyos::suspend_process(972, speedyos::SuspensionType::FULL);
        video::printf_log("SpeedyShell", "Registering hooks for keyboard driver...");

        speedyos::update_status(speedyos::TaskStatus::RUNNING_BUSY);

        speedyos::register_event_for_process(keyboard_pid, 1, on_key_press);
        speedyos::register_event_for_process(keyboard_pid, 4 | 8, on_modifier_event);
        //speedyos::suspend_process(1072, speedyos::SuspensionType::FULL);

        video::printf_log("SpeedyShell", "Registering hooks for scheduler...");
        
        uint32_t scheduler_pid = speedyos::fetch_process_id_by_string("Scheduler");
        speedyos::register_event_for_process(scheduler_pid, 2, on_process_end);

        //speedyos::suspend_process(972, speedyos::SuspensionType::FULL);

        video::printf_log("SpeedyShell", "Successfully registered hooks. Preparing...");
        video::printnl();

        //speedyos::suspend_process(1400, speedyos::SuspensionType::FULL);

        video::clearscr();

        video::printf("Welcome to the SpeedyShell terminal.\n", VGA_COLOUR::WHITE);
        video::printf("Will be improved soon (hopefully).\n\n", VGA_COLOUR::WHITE);

        print_prefix();

        return speedyos::update_status(speedyos::TaskStatus::RUNNING_WAITING_FOR_DATA);

        speedyos::end_process();
    }
}