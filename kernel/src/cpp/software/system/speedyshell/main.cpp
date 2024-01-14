#include "main.h"
#include "stdint.h"
#include "../../../io/video.h"
#include "../../include/sys.h"
#include "../../../drivers/keyboard/keyboard.h"
#include "../../../io/fonts.h"
#include "../../../io/fonts/internal.h"
#include "../../../scheduling/scheduler.h"
#include "../../../misc/conversions.h"
#include "commands.h"
#include "../../../misc/assert.h"

#define WIDTH_WITH_PREFIX 60
#define WIDTH 80

namespace speedyshell {
    uint32_t running_process_id = 0;
    uint32_t input_thread_id = 0;

    char text_buffer[100];
    uint8_t text_buffer_position = 0;

    // Boolean variables.
    bool caps_text = false;
    bool input_mode = false;
    bool allow_typing = true;
    bool pixel_mode = false;

    // Move everything up by 14 pixels to make space if required.
    void safe_y_space() {
        /*if (video::y_offset >= video::VGA_HEIGHT - 28) {
            uint32_t y_start = video::VGA_WIDTH;

            for (uint32_t x = video::VGA_WIDTH * 16; x < video::VGA_WIDTH * video::VGA_HEIGHT; x++) {
                graphics::draw_pixel_linear(y_start, graphics::pixel_colour_linear(x));
                graphics::draw_pixel_linear(x, 0x000000);

                y_start += 1;
            }

            // Start on new line.
            //video::y_offset = video::VGA_HEIGHT - 15;
            //video::x_offset = 0;
        } else video::printnl();*/
    }

    // Reusable functions.
    void printf(char* text, speedyos::VGAColour colour, speedyos::VGAColour back_colour) {
        uint32_t i = 0;

        while (true) {
            char c = text[i];
            
            if (c == '\n') {
                safe_y_space();
            } else if (c == '\0') {
                break;
            } else {
                /*if (
                    video::y_offset > video::VGA_HEIGHT ||
                    font_interpreter::char_width(internal_fonts::bios_port_improved, c) + video::x_offset 
                    > video::VGA_WIDTH
                ) safe_y_space();*/

                video::printf(c, colour);
            }

            i++;
        }
    }

    void clear_buffer() {
        for (uint32_t i = 0; i < text_buffer_position; i++) {
            text_buffer[i] = 0;
        }

        text_buffer_position = 0;
    }

    void print_cursor() {
        video::printf(' ', VGA_COLOUR::LIGHT_GREY, VGA_COLOUR::LIGHT_GREY);
    }

    void print_prefix() {
        video::printf("<SpeedyOS@User>", 0x37ed52);
        video::printf(" $ ", VGA_COLOUR::LIGHT_BLUE);
    }

    void on_process_end(uint32_t id, uint32_t data) {
        if (running_process_id == data) {
            // If running in pixel mode, downgrade.
            if (pixel_mode) {
                // Restore screen.
                //video::restorescr();

                // Update state.
                pixel_mode = false;
            }

            safe_y_space();

            clear_buffer();

            print_prefix();
            print_cursor();

            running_process_id = 0;
            input_mode = false;
            allow_typing = true;
        }

        return speedyos::end_event();
    }

    void on_key_press(uint32_t id, uint32_t data) {
        if (!allow_typing) return speedyos::end_event();
        
        char character;

        if (caps_text) character = drivers::keyboard::ascii_to_uppercase(data);
        else character = data;

        // Check if character will overflow display.
        // To be improved.
        /*if (
            font_interpreter::char_width(internal_fonts::bios_port_improved, character) + video::x_offset 
            > video::VGA_WIDTH
        ) return speedyos::end_event();*/

        // Add key to buffer.
        text_buffer[text_buffer_position] = character;
        text_buffer_position++;

        // Draw on screen.
        //video::printf_reverse(' ');
        video::printf(character);
        
        //if (text_buffer_position + 1 < max_allowed_buffer) 
        print_cursor();

        return speedyos::end_event();
    }

    void on_modifier_press(uint32_t id, uint32_t data) {
        if (data == speedyos::ModifierKeys::CAPSLOCK_PRESSED) {
            caps_text = !caps_text;
        } else if (data == speedyos::ModifierKeys::LEFTSHIFT_PRESSED) {
            caps_text = true;
        } else if (data == speedyos::ModifierKeys::LEFTSHIFT_RELEASED) {
            caps_text = false;
        } else if (data == speedyos::ModifierKeys::ESCAPE_PRESSED) {
            if (running_process_id == 0) return speedyos::end_event();

            Process* process = scheduler::process_list->fetch(running_process_id);
            
            // Suspend process.
            process->state.suspended = true;

            // If graphics mode, restore terminal.
            if (pixel_mode) {
                //video::restorescr();
                pixel_mode = false;
            }

            //if (allow_typing) video::printf_reverse(' ');

            safe_y_space();

            running_process_id = 0;
            input_mode = false;

            structures::string t = "[1] ";
            t.concat(conversions::s_int_to_char(process->id));
            t.concat(" suspended  ");
            t.concat(process->name);

            printf(t, speedyos::VGAColour::WHITE);

            // Terminate the program.
            scheduler::kill_process(process, 1);

            safe_y_space();

            clear_buffer();
            print_prefix();
            print_cursor();

            allow_typing = true;

            delete t;
            
            speedyos::end_event();    
        } else if (!allow_typing) {
            return speedyos::end_event();    
        } else if (data == speedyos::ModifierKeys::ENTER_PRESSED) {
            // Remove cursor.
            //if (allow_typing) video::printf_reverse(' ');

            if (input_mode) {
                Thread* thread = scheduler::thread_list->fetch(input_thread_id);

                // Unsuspend thread.
                thread->state.suspended = false;
                thread->suspension_details.resume_time = 0;

                // Disable input mode.
                input_mode = false;
                allow_typing = false;
                input_thread_id = 0;

                // Re-schedule process.
                scheduler::thread_execution_queue->push(thread);

                safe_y_space();
            } else {
                safe_y_space();

                // Handle commands.
                speedyshell::handle_command();

                // Clear input.
                clear_buffer();

                // Prepare new line.
                print_prefix();
                print_cursor();
            }

        } else if (data == speedyos::ModifierKeys::BACKSPACE_PRESSED) {
            if (text_buffer_position == 0) return speedyos::end_event();

            // Remove character from buffer.
            text_buffer_position--;

            // Remove two sections from screen.
            //video::printf_reverse(' ');
            //video::printf_reverse(text_buffer[text_buffer_position]);

            // Reset character in buffer.
            text_buffer[text_buffer_position] = 0;

            // Print prefix.
            print_cursor();
        }

        return speedyos::end_event();
    }

    void start() {
        speedyos::suspend_thread(4000);

        // Find keyboard PID and register event handlers.
        uint32_t keyboard_pid = speedyos::fetch_process_id_by_string("Keyboard Driver");

        // Register events.
        speedyos::register_event_for_thread(keyboard_pid, 1, on_key_press);
        speedyos::register_event_for_thread(keyboard_pid, 4 | 8, on_modifier_press);

        // Hook scheduler for process events.
        uint32_t scheduler_pid = speedyos::fetch_process_id_by_string("Scheduler Events");

        speedyos::register_event_for_thread(scheduler_pid, 2, on_process_end);

        // Clear screen.
        video::clearscr(VGA_COLOUR::BLACK);

        // Print cursor and text.
        print_prefix();
        print_cursor();

        // Switch to event mode.
        return speedyos::update_execution_policy(speedyos::ThreadExecutionPolicy::EVENT_ONLY);
    }
}