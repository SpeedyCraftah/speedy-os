#include "str.h"
#include "../heap/kernelalloc.h"
#include "../io/video.h"
#include "conversions.h"

int str::length(char* chars) {
    int i = 0;

    while (1) {
        if (chars[i] == '\0') break;
        i++;
    }

    return i;
}   

char* str::join_some(char delim, char* a, char* b, char* c) {
    bool use_delim = delim != '\0';

    int a_size = length(a);
    int b_size = length(b);
    int c_size = length(c);

    int alloc_size = 1 + a_size + b_size + use_delim;
    if (c_size != 0) alloc_size += c_size + use_delim;

    char* text = (char*)kmalloc(alloc_size);
    int text_i = 0;

    for (int i = 0; i < a_size; i++) {
        text[text_i] = a[i];
        text_i++;
    }

    if (use_delim) {
        text[text_i] = delim;
        text_i++;
    }

    for (int i = 0; i < b_size; i++) {
        text[text_i] = b[i];
        text_i++;
    }

    if (c_size != 0) {
        if (use_delim) {
        text[text_i] = delim;
        text_i++;
        }

        for (int i = 0; i < c_size; i++) {
            text[text_i] = c[i];
            text_i++;
        }
    }

    kfree(text);

    return text;
}