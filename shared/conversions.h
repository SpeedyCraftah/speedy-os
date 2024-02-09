#pragma once

#include "string.h"
#include "smart_ptr.h"
#include "stdint.h"

#define HEX32_STR_SIZE (sizeof(uint32_t) * 2) + 3

namespace conversions {
    char* s_int_to_char(int i);
    char* u_int_to_char(uint64_t i);
    
    char* u_int32_to_hex(uint32_t num, char* out);

    int char_to_s_int(char* text);
    bool char_valid_s_int(char* text);
    
    int u_char_to_int(char text);

    smart_ptr<char> s_float_to_char(float i, uint32_t precision, bool roundLastImprecise);

    char* ibytes_friendly_to_char(int bytes);

    float extract_fraction(float num);
}
