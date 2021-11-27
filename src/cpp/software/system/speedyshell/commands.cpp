#include "commands.h"
#include "main.h"
#include "../../../misc/conversions.h"
#include "../../../abstractions/cpu.h"
#include "../../include/sys.h"
#include "../../../abstractions/io_port.h"
#include "../../../scheduling/scheduler.h"

#include "../../utils/info/main.h"

using namespace structures;

namespace speedyshell {
    void handle_command() {
        if (!get_text_input_position()) return;

        string input = get_text_input();

        if (input.hash_equal_to("heap")) {
            string allocated_bytes = "Reserved heap bytes: ";
            string percent_reserved = "Percentage of reserved blocks: ";
            string percent_used = "Percentage of used heap: ";
            string total_blocks = "Total heap blocks: ";
            string available_heap_bytes = "Available heap bytes: ";

            allocated_bytes.concat(conversions::s_int_to_char(heap::get_allocated_bytes()))
                .concat(" byte(s)");

            total_blocks.concat(conversions::s_int_to_char(heap::get_total_blocks()));

            float pcnt_rsvd = ((float)heap::get_reserved_blocks() / heap::get_total_blocks()) * 100;
            float pcnt_used = ((float)heap::get_allocated_bytes() / 1e+8) * 100;

            percent_reserved.concat(conversions::s_int_to_char((uint32_t)pcnt_rsvd))
                .concat("%");

            percent_used.concat(conversions::s_int_to_char((uint32_t)pcnt_used))
                .concat("%");

            available_heap_bytes.concat(conversions::s_int_to_char(1e+8 - heap::get_allocated_bytes()))
                .concat(" byte(s)");

            printf(allocated_bytes);
            printf(available_heap_bytes);
            printf(percent_used);
            printf(percent_reserved);
            printf(total_blocks);
        } else if (input.hash_equal_to("time")) {
            string str("Elapsed scheduler time since startup is ");
            
            str.concat(conversions::s_int_to_char(speedyos::fetch_elapsed_time()));
            str.concat("ms.");

            printf(str);
        } else if (input.starts_with("split")) {
            auto f = input.split_by(' ');
            if (f.get_size() <= 1) return printf("Please specify some args.");

            string str = "Items: ";

            for (uint32_t i = 1; i < f.get_size(); i++) {
                str.concat(f.get_at(i));
                if (i != f.get_size() - 1) str.concat(", ");
            }

            printf(str);

        } else if (input.starts_with("add")) {
            auto args = input.split_by(' ');

            if (args.get_size() != 3) return printf("Too many or too little arguments provided.", VGA_COLOUR::LIGHT_RED);

            if (!conversions::char_valid_s_int(args[1]) || !conversions::char_valid_s_int(args[2]))
                return printf("One or more arguments is not a number.", VGA_COLOUR::LIGHT_RED);

            uint32_t a = conversions::char_to_s_int(args[1]);
            uint32_t b = conversions::char_to_s_int(args[2]);

            uint32_t result = a + b;

            string s;

            s.concat(args[1]).concat(" + ").concat(args[2])
                .concat(" = ").concat(conversions::s_int_to_char(result)).concat(".");

            printf(s);
        } else if (input.starts_with("mul")) {
            auto args = input.split_by(' ');

            if (args.get_size() != 3) return printf("Too many or too little arguments provided.", VGA_COLOUR::LIGHT_RED);

            if (!conversions::char_valid_s_int(args[1]) || !conversions::char_valid_s_int(args[2]))
                return printf("One or more arguments is not a number.", VGA_COLOUR::LIGHT_RED);

            uint32_t a = conversions::char_to_s_int(args[1]);
            uint32_t b = conversions::char_to_s_int(args[2]);

            uint32_t result = a * b;

            string s;

            s.concat(args[1]).concat(" x ").concat(args[2])
                .concat(" = ").concat(conversions::s_int_to_char(result)).concat(".");

            printf(s);
        } else if (input.starts_with("sub")) {
            auto args = input.split_by(' ');

            if (args.get_size() != 3) return printf("Too many or too little arguments provided.", VGA_COLOUR::LIGHT_RED);

            if (!conversions::char_valid_s_int(args[1]) || !conversions::char_valid_s_int(args[2]))
                return printf("One or more arguments is not a number.", VGA_COLOUR::LIGHT_RED);

            uint32_t a = conversions::char_to_s_int(args[1]);
            uint32_t b = conversions::char_to_s_int(args[2]);

            uint32_t result = a - b;

            string s;

            s.concat(args[1]).concat(" - ").concat(args[2])
                .concat(" = ").concat(conversions::s_int_to_char(result)).concat(".");

            printf(s);
        } else if (input.hash_equal_to("shutdown")) {
            video::clearscr(VGA_COLOUR::BLACK);

            video::printf("Machine frozen.", VGA_COLOUR::WHITE);
            video::printnl();
            video::printf("It is now safe to switch off the machine.", VGA_COLOUR::WHITE);

            asm volatile("cli");
            asm volatile("hlt");
        } else if (input.hash_equal_to("info")) {
            running_process_id = scheduler::start_process("Info", software::info::start, TaskStatus::RUNNING, 0, true, true);
            return speedyos::end_event();
        } else printf("Unknown command entered.");
    }
}