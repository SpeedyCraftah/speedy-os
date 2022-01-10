#include "fonts.h"

namespace font_interpreter {
    smart_ptr<uint16_t> resize_char_nn(uint16_t* font, uint8_t target_width, uint8_t target_height) {
        uint16_t* temp = (uint16_t*)heap::malloc(target_width * target_height * sizeof(uint16_t));
    
        uint16_t width = font[0];
        uint16_t height = font[1];

        // Skip over meta data.
        font = font + 2;

        double x_ratio = width /( double)target_width;
        double y_ratio = height / (double)target_height;

        double px, py; 
        for (int i = 0; i < target_height; i++) {
            for (int j = 0; j < target_width; j++) {
                px = uint16_t(j * x_ratio);
                py = uint16_t(i * y_ratio);

                temp[(i * target_width) + j] = font[(int)((py * width) + px)];
            }
        }

        return smart_ptr<uint16_t>(temp);
    }

    uint16_t* load_char(uint16_t* font, uint8_t character) {
        // If character is not defined in pack, use default.
        uint16_t index = character > font[0] ? 0 : character;

        // Lookup character in index.
        uint16_t* data = &font[font[index + 1]];

        // If character is empty.
        if (data[0] == 0 && data[1] == 0) {
            // Use default.
            // Font 0 = default font, never empty.
            data = &font[font[1]];
        }

        // Return data.
        return data;
    }

    uint8_t char_width(uint16_t* font, uint8_t character) {
        // Lookup index.
        // If character is not defined in pack, use default.
        uint16_t index = character > font[0] ? 0 : character;

        // Lookup character in index.
        uint16_t* data = &font[font[index + 1]];

        // If character is empty.
        if (data[0] == 0 && data[1] == 0) {
            // Use default.
            // Font 0 = default font, never empty.
            data = &font[font[1]];
        }

        // Return width.
        return data[0];
    }

    uint8_t char_height(uint16_t* font, uint8_t character) {
        // Lookup index.
        // If character is not defined in pack, use default.
        uint16_t index = character > font[0] ? 0 : character;

        // Lookup character in index.
        uint16_t* data = &font[font[index + 1]];

        // If character is empty.
        if (data[0] == 0 && data[1] == 0) {
            // Use default.
            // Font 0 = default font, never empty.
            data = &font[font[1]];
        }

        // Return height.
        return data[1];
    }
}