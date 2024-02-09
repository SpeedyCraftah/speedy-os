#include "_shared.h"
#include "conversions.h"
#include "smart_ptr.h"
#include "stdint.h"
#include "string.h"
#include "str.h"

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

char* conversions::u_int_to_char(uint64_t i) {
    static char text[12];
    text[11] = 0;

    int loc = 11;

    while (i) {
        text[--loc] = '0' + (i % 10);
        i /= 10;
    }

    if (loc == 11) text[--loc] = '0';

    return &text[loc];
}

char* conversions::u_int32_to_hex(uint32_t num, char* out) {
    static char hex[] = "0123456789ABCDEF";

    out[0] = '0';
    out[1] = 'x';

    #pragma unroll
    for (int i = 0; i < (sizeof(uint32_t) * 2); i++) {
        uint8_t nibble = (num >> (28 - (i * 4))) & 0xF;
        out[i + 2] = hex[nibble];
    }

    out[HEX32_STR_SIZE - 1] = '\0';
    return out;
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

inline float conversions::extract_fraction(float num) {
    return num - (uint32_t)num;
}

smart_ptr<char> conversions::s_float_to_char(float num, uint32_t precision, bool roundLastImprecise) {
    // Setup string.
    char* string = (char*)_shared_malloc(20);
    uint32_t string_i = 0;

    char* whole_number = s_int_to_char((uint32_t)num);

    // Copy whole number to string.
    while (whole_number[string_i] != '\0') {
        string[string_i] = whole_number[string_i];
        string_i++;
    }

    // Add decimal point.
    string[string_i] = '.';
    string_i++;

    // Get decimal part of number.
    float decimal = extract_fraction(num);

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
    char* decimal_number = s_int_to_char(decimal_int);
    uint32_t decimal_i = 0;

    while (decimal_number[decimal_i] != '\0') {
        string[string_i] = decimal_number[decimal_i];
        string_i++;
        decimal_i++;
    }

    // Add terminator.
    string[string_i] = '\0';

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
