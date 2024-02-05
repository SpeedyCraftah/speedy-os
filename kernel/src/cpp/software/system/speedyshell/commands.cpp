#include "commands.h"
#include "main.h"
#include "../../../../../../shared/conversions.h"
#include "../../../abstractions/cpu.h"
#include "../../include/sys.h"
#include "../../../abstractions/io_port.h"
#include "../../../scheduling/scheduler.h"

/*#include "../../utils/info/main.h"
#include "../../random/main.h"
#include "../../games/guess/main.h"
#include "../../uuid/main.h"
#include "../../graphics/colors.h"
#include "../../mouse/mouse.h"
#include "../../thread/thread.h"
#include "../../games/speedysnake/main.h"
#include "../../utils/stress/main.h"
#include "../../sort/main.h"*/
#include "../../../acpi/acpi.h"
#include "../../../misc/assert.h"

using namespace structures;

namespace speedyshell {
    void safe_heap_handle_command() {
        if (speedyshell::text_buffer_position == 0) return;

        string input = string(speedyshell::text_buffer);
        auto args = input.split_by(' ');

        /*if (string("info").hash_equal_to(args[0])) {
            running_process_id =
                scheduler::create_process("Info", software::info::start)->id;
        }
        
        else if (string("random").hash_equal_to(args[0])) {
            running_process_id =
                scheduler::create_process("Random", software::random::start)->id;
        }

        else if (string("guess").hash_equal_to(args[0])) {
            running_process_id =
                scheduler::create_process("Guess", software::guess::start)->id;
        }

        else if (string("uuid") == args[0]) {
            running_process_id =
                scheduler::create_process("UUID", software::uuid::start)->id;
        }

        else if (string("graphics") == args[0]) {
            running_process_id =
                scheduler::create_process("Graphics", software::colors::start)->id;
        } else if (string("mouse") == args[0]) {
            running_process_id =
                scheduler::create_process("Mouse", software::mouse::start)->id;
        } else if (string("thread") == args[0]) {
            running_process_id =
                scheduler::create_process("Thread", software::thread::start)->id;
        } else if (string("snake") == args[0]) {
            running_process_id = 
                scheduler::create_process("Snake", software::snake::start)->id;
        } else if (string("stress") == args[0]) {
            running_process_id = 
                scheduler::create_process("Stress", software::stress::start)->id;
        } else if (string("sort") == args[0]) {
            running_process_id = 
                scheduler::create_process("Sort", software::sort::start)->id;
        } else if (string("shutdown") == args[0]) {
            printf("Goodbye chief.");
            acpi::enter_sleep_state(acpi::sleep_type::S5_MECHANICALOFF);
        }*/

        
        // else printf("Command not found.\n");
    }

    void handle_command() {
        // Call another method as C++ will otherwise refuse to deallocate objects.
        safe_heap_handle_command();

        if (running_process_id) {
            allow_typing = false;

            return speedyos::end_event();
        }
    }
}