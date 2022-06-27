#pragma once

#include "stdint.h"
#include "fonts.h"

// Graphics manipulation software, inspired partially by HTML canvas.

struct rgb_colour {
  rgb_colour(uint8_t red, uint8_t green, uint8_t blue) {
    r = red;
    g = green;
    b = blue;
  }

  rgb_colour(uint32_t hex) {
    b = hex & 255;
    g = (hex >> 8) & 255;
    r = (hex >> 16) & 255;
  }

  rgb_colour() {}

  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Left for compatibility reasons.
enum VGA_COLOUR : uint32_t {
    BLACK = 0x000000,
    BLUE = 0x0000FF,
    GREEN = 0x00FF00,
    CYAN = 0x00FFFF,
    RED = 0xFF0000,
    MAGENTA = 0xFF00FF,
    BROWN = 0xA52A2A,
    LIGHT_GREY = 0xD3D3D3,
    DARK_GREY = 0x808080,
    LIGHT_BLUE = 0x00b3ff,
    LIGHT_GREEN = 0x90EE90,
    LIGHT_CYAN = 0xE0FFFF,
    LIGHT_RED = 0xFFA07A,
    LIGHT_MAGENTA = 0xFFE4E1,
    LIGHT_BROWN = 0xFFDEAD,
    WHITE = 0xFFFFFF
};

// Functions with likely high amount of calls are inlined to reduce function call overhead.

namespace graphics {
  extern uint32_t* video_address;
  extern uint32_t resolution_width;
  extern uint32_t resolution_height;

  extern uint32_t fill_colour;
  extern uint32_t outline_colour;
  extern uint32_t outline_width;
  
  extern uint32_t double_buffer[800 * 600];

  // Plots a single pixel at the x and y coordinates.
  inline void __attribute__((always_inline)) draw_pixel(uint32_t x, uint32_t y, uint32_t colour) {
    uint32_t offset = resolution_width * y + x;

    *(video_address + offset) = colour;
    *(double_buffer + offset) = colour;
  }

  inline void __attribute__((always_inline)) draw_pixel_linear(uint32_t x, uint32_t colour) {
    *(video_address + x) = colour;
    *(double_buffer + x) = colour;
  }

  inline uint32_t __attribute__((always_inline)) pixel_colour(uint32_t x, uint32_t y) {
    return *(double_buffer + resolution_width * y + x);
  }

  inline uint32_t __attribute__((always_inline)) pixel_colour_linear(uint32_t x) {
    return *(double_buffer + x);
  }

  // Computes the width of the text in pixels.
  uint32_t compute_text_width(uint16_t* font, char* str, float scale = 1);

  void draw_text(uint16_t* font, uint32_t x_start, uint32_t y_start, char* str, float scale = 1);

  // Draws a string of any size.
  template <typename CMF>
  void draw_text_mod(uint16_t* font, uint32_t x_start, uint32_t y_start, char* str, CMF colour_modifier, float scale = 1) {
    uint32_t i = 0;
    uint32_t offset_x = 0;
    uint32_t offset_y = 0;

    while (true) {
      uint8_t character = str[i];

      if (character == 0) {
        break;
      } else if (character == '\n') {
        // Get width of likely highest character (all characters have same height right now).
        uint8_t height_offset = font_interpreter::char_height(font, '!') * scale;

        // Add offset.
        offset_y += height_offset + 1;

        // Reset X.
        offset_x = 0;

        i++;
        continue;
      }

      // Load character.
      uint16_t* data = font_interpreter::load_char(font, str[i]);

      uint16_t width = data[0];
      uint16_t height = data[1];

      auto resized_data = font_interpreter::resize_char_nn(
          data, width * scale, height * scale
      ).ptr();

      // Take in a custom colour modifier.
      uint32_t colour = colour_modifier(character, i);

      for (int y = 0; y < height * scale; y++) {
          uint32_t y_value = y + y_start + offset_y;
          uint32_t index = (y * (width * scale));

          for (int x = 0; x < width * scale; x++) {
              if (resized_data[index + x] == 1) {
                graphics::draw_pixel(x_start + x + offset_x, y_value, colour);
              } 
          }
      }

      offset_x += 2 + width * scale;

      i++;
    }
  }

  // Draws a non-straight line using the Bresenham algorithm.
  void draw_line_bresenham(uint32_t start_x, uint32_t start_y, uint32_t end_x, uint32_t end_y);
  
  // Draws a rectangle. Outline width will be taken into account in the future.
  // X - Top left corner coordinate. Y - Top left corner Y coordinate. Length - Length in pixels from corner points.
  void draw_rectangle(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill = false);
  
  // Draws a rectangle. Same as above except colour will be taken from the location at the buffer.
  void draw_rectangle_from(uint32_t* buffer, uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill = false);

  // Draws a line, straight or diagonal.
  // Width will be taken into account in the future.
  void draw_line(uint32_t x, uint32_t y, uint32_t length, bool vertical);
}