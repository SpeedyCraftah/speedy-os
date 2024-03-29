#include "alloc.h"
#include "sys.h"

// Stack protector definitions.
uint32_t __stack_chk_guard = 0xd3adb33f;
extern "C" void __stack_chk_fail(void) {
    speedyos::speedyshell::printf("[FATAL] Stack protector detected change to canary");
    speedyos::end_process(1);
    __builtin_unreachable();
}

extern "C" void (*_call_ctors)();
extern "C" void (*_call_dtors)();

extern "C" int main();
extern "C" void __attribute__((no_stack_protector)) _main() {
    // Randomize the stack canary (current MS time for now).
    __stack_chk_guard = speedyos::fetch_elapsed_time();

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