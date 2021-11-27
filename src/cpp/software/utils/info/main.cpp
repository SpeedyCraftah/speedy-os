#include "main.h"

#include "../../system/speedyshell/main.h"
#include "../../include/sys.h"
#include "../../../structures/string.h"
#include "../../../misc/conversions.h"

using namespace structures;

void software::info::start() {
    string input = speedyos::speedyshell::fetch_input();
    auto args = input.split_by(' ');

    if (args.get_size() == 1) {
        speedyos::speedyshell::printf("Use this tool to gather information about your system.");
    } else if (string("heap").hash_equal_to(args[1])) {
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

        percent_reserved.concat(conversions::s_double_to_char(pcnt_rsvd, 1, true))
            .concat("%");

        percent_used.concat(conversions::s_double_to_char(pcnt_used, 2, true))
            .concat("%");

        available_heap_bytes.concat(conversions::s_int_to_char(1e+8 - heap::get_allocated_bytes()))
            .concat(" byte(s)");

        speedyos::speedyshell::printf(allocated_bytes);
        speedyos::speedyshell::printf(available_heap_bytes);
        speedyos::speedyshell::printf(percent_used);
        speedyos::speedyshell::printf(percent_reserved);
        speedyos::speedyshell::printf(total_blocks);
    }

    return speedyos::end_process();
}