#include "events.h"

#include "../../include/sys.h"
#include "main.h"

void __attribute__((fastcall)) speedyshell::on_process_end(uint32_t id, uint32_t data) {
    if (running_process_id == data) {
        clear_input();
        print_prefix();

        running_process_id = 0;
    }

    speedyos::end_event();
}