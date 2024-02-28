#include "stdint.h"

// A separate system call header for specialized applications (e.g. interface providers).
namespace speedyos {
    // Contains system calls to be used by interface providers only.
    namespace interface_provider {
        // Requests the kernel to return the ID of the output data sink of the interface provider, if it exists.
        __attribute__((naked)) __attribute__((fastcall)) __attribute__((no_stack_protector)) uint32_t fetch_output_sink_id();
    }
}