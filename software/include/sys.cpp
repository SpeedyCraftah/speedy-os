#include "sys.h"
#include "../../shared/str.h"

namespace speedyos {
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

    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_elapsed_time() {
        asm volatile("mov $1, %ecx");
        asm volatile("xor %edx, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_graphics_resolution() {
        asm volatile("mov $1, %ecx");
        asm volatile("mov $3, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_colour_depth() {
        asm volatile("mov $1, %ecx");
        asm volatile("mov $4, %edx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) __attribute__((noreturn)) void end_process(uint32_t code) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $2, %ecx");
        asm volatile("int $128");
        asm volatile("ud2");
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

    __attribute__((naked)) __attribute__((fastcall)) uint32_t _fetch_process_id_by_string(char* process_name, uint32_t process_name_length) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $10, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    uint32_t fetch_process_id_by_string(char* process_name) {
        return _fetch_process_id_by_string(process_name, str::length(process_name));
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t hardware_random() {
        asm volatile("mov $14, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) uint32_t create_thread(void (*start)(void* capture), void* capture) {
        asm volatile("mov %edx, %eax");
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

    __attribute__((naked)) __attribute__((fastcall)) uint32_t* upgrade_graphics() {
        asm volatile("mov $19, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void preempt_thread() {
        asm volatile("mov $20, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) void* alloc_virtual_pages(void* requested_page, uint32_t count, uint32_t flags) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $21, %ecx");

        // Preserve ebx register as required by ABI.
        asm volatile("push %ebx");
        asm volatile("mov 8(%esp), %ebx");

        asm volatile("int $128");

        // Restore ebp register.
        asm volatile("pop %ebx");
        
        asm volatile("ret $4");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool free_virtual_page(void* address, uint32_t flags) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $22, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) bool write_steady_datasink(uint32_t sink_id, uint8_t* data, uint32_t data_size) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $23, %ecx");

        // Preserve ebx register as required by ABI.
        asm volatile("push %ebx");
        asm volatile("mov 8(%esp), %ebx");

        asm volatile("int $128");

        // Restore ebp register.
        asm volatile("pop %ebx");
        
        asm volatile("ret $4");
    }

    __attribute__((naked)) __attribute__((fastcall)) int read_steady_datasink(uint32_t sink_id, uint8_t* dest, uint32_t read_size) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $24, %ecx");

        // Preserve ebx register as required by ABI.
        asm volatile("push %ebx");
        asm volatile("mov 8(%esp), %ebx");

        asm volatile("int $128");

        // Restore ebp register.
        asm volatile("pop %ebx");
        
        asm volatile("ret $4");
    }

    __attribute__((naked)) __attribute__((fastcall)) int fetch_fragment_size_steady_datasink(uint32_t sink_id) {
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $25, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    __attribute__((naked)) __attribute__((fastcall)) int read_fragment_steady_datasink(uint32_t sink_id, uint8_t* dest) {
        asm volatile("mov %edx, %eax");
        asm volatile("mov %ecx, %edx");
        asm volatile("mov $26, %ecx");
        asm volatile("int $128");
        asm volatile("ret");
    }

    namespace speedyshell {
        __attribute__((naked)) __attribute__((fastcall)) char* fetch_input() {
            asm volatile("mov $11, %ecx");
            asm volatile("xor %edx, %edx");
            asm volatile("int $128");
            asm volatile("ret");
        }

        __attribute__((naked)) __attribute__((fastcall)) void _printf(char* text, VGAColour colour, uint32_t length) {
            asm volatile("mov %edx, %eax");
            asm volatile("mov %ecx, %edx");
            asm volatile("mov $12, %ecx");

            // Preserve ebx register as required by ABI.
            asm volatile("push %ebx");
            asm volatile("mov 8(%esp), %ebx");

            asm volatile("int $128");

            // Restore ebp register.
            asm volatile("pop %ebx");
            
            asm volatile("ret $4");
        }

        __attribute__((fastcall)) void printf(char* text, VGAColour colour) {
            _printf(text, colour, str::length(text));
        }

        __attribute__((naked)) __attribute__((fastcall)) void request_input() {
            asm volatile("mov $13, %ecx");
            asm volatile("int $128");
            asm volatile("ret");
        }
    }

    void __attribute__((noreturn)) panic(char* error) {
        speedyshell::printf("[FATAL] ");
        speedyshell::printf(error);
        speedyshell::printf("\n");
        speedyos::end_process(1);
        __builtin_unreachable();
    }
}