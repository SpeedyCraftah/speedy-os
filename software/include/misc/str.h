#pragma once

#include "stdint.h"

namespace str {
    // Returns the length of the provided char array (not including terminator).
    uint32_t length(char* chars);

    // Allocates string on the heap then deallocates on return. Designed to be used quickly.
    char* join_some(char delim, char* a, char* b, char* c = "\0");
}