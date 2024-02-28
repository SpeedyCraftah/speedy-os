#include "sys_additional.h"

namespace speedyos {
    namespace interface_provider {
        __attribute__((naked)) __attribute__((fastcall)) __attribute__((no_stack_protector)) uint32_t fetch_output_sink_id() {
            asm volatile("mov $1, %ecx");
            asm volatile("mov $5, %edx");
            asm volatile("int $128");
            asm volatile("ret");
        }
    }
}