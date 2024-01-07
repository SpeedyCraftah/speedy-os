#include "algorithm.h"

namespace algorithm {
    uint32_t hash_string_fnv1a(char* key) {
        uint32_t fnvPrime = 16777619;
        uint32_t hash = 2166136261;

        uint32_t i = 0;

        while (1) {
            if (key[i] == '\0') break;

            uint32_t characterCode = (int)key[i];

            hash ^= characterCode;
            hash *= fnvPrime;

            i++;
        }

        return hash;
    }
}