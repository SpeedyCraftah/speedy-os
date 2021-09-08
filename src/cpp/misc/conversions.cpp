#include "conversions.h"

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

char* conversions::ibytes_friendly_to_char(int bytes) {
    int kb = bytes >> 10;
    int mb = kb >> 10;

    if (bytes == 0) return "0 byte(s)";
    else if (kb == 0) return "as";

    return "not complete";
}
