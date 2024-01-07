#include "alloc.h"
#include "sys.h"

extern "C" void _init();

extern "C" volatile int debug_vb;
extern "C" int main();
extern "C" void _main() {
    // Initialise the malloc heap.
    heap::init();
    
    _init();

    // Call the constructors.
    //_init();
    /*constructor_t* constructor;
    for (constructor = __init_array_start; constructor < __init_array_end; constructor++) {
        (*constructor)();
    }*/

    // Hand over control to program main.
    int code = main();

    // End the process.
    return speedyos::end_process(code);
}