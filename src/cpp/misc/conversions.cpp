#include "conversions.h"

#include "stdint.h"
#include "../structures/string.h"
#include "../misc/str.h"

char* conversions::s_int_to_char(int i) {
    static char text[12];
    text[11] = 0;

    int loc = 11;
    char neg = 1;

    if (i >= 0) {
        neg = 0;
        i = -i;
    }

    while (i) {
        text[--loc] = '0' - (i % 10);
        i /= 10;
    }

    if (loc == 11) text[--loc] = '0';
    if (neg) text[--loc] = '-';
    
    return &text[loc];
}

inline int conversions::u_char_to_int(char text) {
    // If char outside of ascii number ranges, return -1.
    if (text > 57 || text < 48) return -1;

    // Numbers start at ascii 48 so simply subtract.
    return text - 48;
}

bool conversions::char_valid_s_int(char* text) {
    // Safety check.
    if (text[0] == '\0') return false;

    // Find start of number in human form (right to left, excl terminator).
    uint32_t number_start = str::length(text) - 1;

    // Another safety check.
    if (number_start == 0 && text[0] == '-') return false;

    // Safety check for overflow (not complete prevention).
    if (number_start > 10) return false;

    uint32_t i = number_start;

    while (true) {
        if (i == 0 && text[i] == '-') {
            break;
        }

        uint32_t number = u_char_to_int(text[i]);
        if (number == -1) return false;

        if (i == 0) break;
        i--;
    }

    return true;
}

int conversions::char_to_s_int(char* text) {
    // Safety check.
    if (text[0] == '\0') return 0;

    // Find start of number in human form (right to left, excl terminator).
    uint32_t number_start = str::length(text) - 1;
    
    // Another safety check.
    if (number_start == 0 && text[0] == '-') return 0;

    uint32_t tenth_multiplier = 1;

    int total_number = 0;

    uint32_t i = number_start;

    while (true) {
        if (i == 0 && text[i] == '-') {
            total_number *= -1;
            break;
        }

        uint32_t number = u_char_to_int(text[i]);
        if (number == -1) return 0;

        total_number += number * tenth_multiplier;

        tenth_multiplier *= 10;

        if (i == 0) break;
        i--;
    }

    return total_number;
}

inline double conversions::extract_fraction(double num) {
    return num - (uint32_t)num;
}

structures::string conversions::s_double_to_char(double num, uint32_t precision, bool roundLastImprecise) {
    // Setup string.
    structures::string string;

    // Add integer part to string.
    string.concat(s_int_to_char((uint32_t)num))
        .concat(".");

    // Get decimal part of number.
    double decimal = extract_fraction(num);

    uint32_t i = 0;

    // Keep multiplying by 10 until zero is reached, or desired precision is reached.
    while (extract_fraction(decimal) != 0 && i < precision + roundLastImprecise) {
        i++;
        decimal *= 10;
    }

    uint32_t decimal_int = (uint32_t)decimal;

    // If unrounded.
    if (roundLastImprecise && extract_fraction(decimal) != 0) {
        uint32_t rounder = decimal_int % 10;

        // Remove last digit.
        decimal_int /= 10;

        if (rounder >= 5) decimal_int += 1;
    }

    // Add floating part to string.
    string.concat(s_int_to_char(decimal_int));

    // Copy string and return smart pointer.
    return string;
}

char* conversions::ibytes_friendly_to_char(int bytes) {
    int kb = bytes >> 10;
    int mb = kb >> 10;

    if (bytes == 0) return "0 byte(s)";
    else if (kb == 0) return "as";

    return "not complete";
}
