#include "alloc.h"
#include "sys.h"

// Stack protector definitions.
uint32_t __stack_chk_guard = 0x38325789;
extern "C" void __stack_chk_fail(void) {
    speedyos::speedyshell::printf("[FATAL] Stack protector detected change to canary");
    speedyos::end_process();
    __builtin_unreachable();
}

extern "C" void (*_call_ctors)();
extern "C" void (*_call_dtors)();

__attribute__((naked)) __attribute__((fastcall)) void func() {
    asm volatile("mov $0, %ecx");
    asm volatile("int $128");
    asm volatile("ret");
}

extern "C" int main();
extern "C" void _main() {
    // Initialise the malloc heap.
    heap::init();

    // Call the constructors (if they exist).
    if (_call_ctors != _call_dtors) _call_ctors();

    // Hand over control to program main.
    int code = main();

    // Call the destructors (if they exist).
    if (_call_ctors != _call_dtors) _call_dtors();

    // End the process.
    return speedyos::end_process(code);
}