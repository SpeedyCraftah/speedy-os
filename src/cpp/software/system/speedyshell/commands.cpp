#include "commands.h"
#include "main.h"
#include "../../../misc/conversions.h"
#include "../../../abstractions/cpu.h"
#include "../../include/sys.h"
#include "../../../abstractions/io_port.h"
#include "../../../scheduling/scheduler.h"

#include "../../utils/info/main.h"
#include "../../random/main.h"

using namespace structures;

namespace speedyshell {
    void safe_heap_handle_command() {
        if (!get_text_input_position()) return;

        string input = string(get_text_input());
        auto args = input.split_by(' ');

        if (string("info").hash_equal_to(args[0])) {
            running_process_id =
                scheduler::start_process("Info", software::info::start, TaskStatus::RUNNING, 0, false, false);
        }
        
        else if (string("random").hash_equal_to(args[0])) {
            running_process_id =
                scheduler::start_process("Random", software::random::start, TaskStatus::RUNNING, 0, false, false);
        }

        else printf("Command not found.");
    }

    void handle_command() {
        // Call another method as C++ will otherwise refuse to deallocate objects.
        safe_heap_handle_command();

        if (running_process_id) return speedyos::end_event();
    }
}