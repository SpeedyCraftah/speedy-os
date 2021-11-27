#pragma once

#include "../structures/string.h"
#include "stdint.h"

namespace conversions {
    char* s_int_to_char(int i);

    int char_to_s_int(char* text);
    bool char_valid_s_int(char* text);
    
    int u_char_to_int(char text);

    structures::string s_double_to_char(double i, uint32_t precision, bool roundLastImprecise);

    char* ibytes_friendly_to_char(int bytes);

    double extract_fraction(double num);
}
