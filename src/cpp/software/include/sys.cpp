#include "sys.h"

namespace speedyos {
    // Queries the kernel for the process ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id() {
        asm volatile("mov $1, %ecx");
        asm volatile("mov $1, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_thread_id() {
        asm volatile("mov $1, %ecx");
        asm volatile("mov $2, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    // Queries the kernel for the elapsed time from startup.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_elapsed_time() {
        asm volatile("mov $1, %ecx");
        asm volatile("mov $0, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void end_process(uint32_t code) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $2, %ecx");
        asm volatile("int $128");
    }

    __attribute__((naked)) __attribute__((fastcall)) void suspend_thread(uint32_t ms) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $3, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void end_event() {
        asm volatile("mov $6, %ecx");
        asm volatile("int $128");

        // Emergency RET in case the program does return.
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void* malloc_heap(uint32_t bytes) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $4, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool free_heap(void* address) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $5, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool register_event_for_thread(
        uint32_t target_thread_id,
        uint32_t enabled_events,
        void(*handler)(uint32_t, uint32_t)
    ) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $7, %ecx");

        // Preserve ebx register as required by ABI.
        asm volatile("push %ebx");
        asm volatile("mov 8(%esp), %ebx");

        asm volatile("int $128");

        // Restore ebp register.
        asm volatile("pop %ebx");
        
        asm volatile("ret $4");
    }

    __attribute__((naked)) __attribute__((fastcall)) void emit_event(
        uint32_t event_id,
        uint32_t event_data
    ) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $8, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void update_execution_policy(
        ThreadExecutionPolicy policy,
        uint32_t thread_id
    ) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $9, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id_by_string(char* process_name) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $10, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t hardware_random() {
        asm volatile("mov $14, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t create_thread(void (*start)()) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $15, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void kill_thread(uint32_t thread_id) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $16, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool park_thread(uint32_t thread_id) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $17, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool awake_thread(uint32_t thread_id) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $18, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    namespace speedyshell {
        __attribute__((naked)) __attribute__((fastcall)) char* fetch_input() {
            asm volatile("mov $11, %ecx");
            asm volatile("mov $0, %edx");
            asm volatile("int $128");
            asm volatile("ret");
        }

        __attribute__((naked)) __attribute__((fastcall)) void printf(char* text, VGAColour colour) {
            asm volatile("mov %edx, %eax");
            asm volatile("mov %ecx, %edx");
            asm volatile("mov $12, %ecx");
            asm volatile("int $128");
            asm volatile("ret");
        }

        __attribute__((naked)) __attribute__((fastcall)) void request_input() {
            asm volatile("mov $13, %ecx");
            asm volatile("int $128");
            asm volatile("ret");
        }
    }
}