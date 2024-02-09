// Contains definitions depending on the source that included it.

#pragma once

#if defined(_SHARED_KERNEL)
    #include "../kernel/src/cpp/panic/panic.h"
    #include "../kernel/src/cpp/heap/kernelalloc.h"

    #define _shared_panic kernel::panic
    #define _shared_malloc kmalloc
    #define _shared_free kfree
#elif defined(_SHARED_USER)
    #include "../software/include/sys.h"
    #include "../software/include/alloc.h"

    #define _shared_panic speedyos::panic
    #define _shared_malloc malloc
    #define _shared_free free
#else
    #error "Static shared library is not sure where it was imported from so the correct functions cannot be defined, define _SHARED_KERNEL or _SHARED_USER during compilation."
#endif