#pragma once

#include "stdint.h"

// General other events.

namespace speedyshell {
    void __attribute__((fastcall)) on_process_end(uint32_t id, uint32_t data);
    //void __attribute__((fastcall)) on_text_print(uint32_t id, uint32_t data);
}