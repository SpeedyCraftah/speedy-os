#include "alloc.h"
#include "sys.h"

extern "C" void (*_call_ctors)();
extern "C" void (*_call_dtors)();

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