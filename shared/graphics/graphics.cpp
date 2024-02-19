#include "graphics.h"

#include "../_shared.h"
#include "stdint.h"
#include "fonts.h"
#include "fonts/internal.h"

namespace graphics {
  uint32_t double_buffer[800 * 600];

  uint32_t* video_address = 0;
  uint32_t resolution_width = 0;
  uint32_t resolution_height = 0;

  uint32_t fill_colour = (uint32_t)0xFFFFFF;
  uint32_t outline_colour = (uint32_t)0xFFFFFF;
  uint32_t outline_width = 1;

  #ifdef _SHARED_USER
    void init(uint32_t* video_addr, uint32_t res, uint32_t colour_depth) {
      video_address = video_addr;
      resolution_width = (res >> 16) & 0xFFFF;
      resolution_height = (res & 0xFFFF);
    }
  #endif
  
  uint32_t draw_text(uint16_t* font, uint32_t x_start, uint32_t y_start, char* str) {
    uint32_t i = 0;
    uint32_t offset_x = 0;
    uint32_t offset_y = 0;

    while (true) {
      uint8_t character = str[i];

      if (character == 0) {
        break;
      } else if (character == '\n') {
        // Get width of likely highest character (all characters have same height right now).
        uint8_t height_offset = font_interpreter::char_height(font, '!');

        // Add offset.
        offset_y += height_offset + 1;

        // Reset X.
        offset_x = 0;

        i++;
        continue;
      }

      // Load character.
      uint16_t* font_data = font_interpreter::load_char(
          internal_fonts::bios_port_improved, str[i]
      );

      uint16_t* data = font_data + 2;

      uint16_t width = font_data[0];
      uint16_t height = font_data[1];

      // Take in a custom colour modifier.
      uint32_t colour = fill_colour;

      for (int y = 0; y < height; y++) {
          uint32_t y_value = y + y_start + offset_y;
          uint32_t index = (y * width);

          for (int x = 0; x < width; x++) {
              if (data[index + x] == 1) {
                graphics::draw_pixel(x_start + x + offset_x, y_value, colour);
              } 
          }
      }

      offset_x += 2 + width;

      i++;
    }

    return offset_x;
  }

  void draw_text_with_scale(uint16_t* font, uint32_t x_start, uint32_t y_start, char* str, float scale) {
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
      uint16_t* data = font_interpreter::load_char(
          internal_fonts::bios_port_improved, str[i]
      );

      uint16_t width = data[0];
      uint16_t height = data[1];

      auto resized_data = font_interpreter::resize_char_nn(
          data, width * scale, height * scale
      );

      // Take in a custom colour modifier.
      uint32_t colour = fill_colour;

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

  uint32_t compute_text_width(uint16_t* font, char* str, float scale) {
    uint32_t i = 0;
    uint32_t total = 0;

    while (true) {
      char character = str[i];
      if (character == '\0') break;

      // Get character width from font interpreter.
      uint32_t width = font_interpreter::char_width(font, character);

      // Add onto total taking scale into account (with x-offset).
      total += (width * scale) + 2;

      i++;
    }

    // Return total.
    return total;
  }
  
  void draw_line(uint32_t x, uint32_t y, uint32_t length, bool vertical) {
    if (vertical) {
      for (uint32_t j = 0; j < outline_width; j++) {
        for (uint32_t i = 0; i < length; i++) draw_pixel(x + i, y + j, outline_colour);
      }
    } else {
      for (uint32_t j = 0; j < outline_width; j++) {
        for (uint32_t i = 0; i < length; i++) draw_pixel(x + j, y + i, outline_colour);
      }
    }
  }

  // Draws a line from start to end using the Bresenham line algorithm.
  void draw_line_bresenham(uint32_t start_x, uint32_t start_y, uint32_t end_x, uint32_t end_y) {
    // Calculate deltas.
    int32_t delta_x = end_x - start_x;
    int32_t delta_y = end_y - start_y;

    // Calculate error.
    int32_t error = 0;

    // Calculate step.
    int32_t step_x = 1;
    int32_t step_y = 1;

    if (delta_x < 0) {
      step_x = -1;
      delta_x = -delta_x;
    }

    if (delta_y < 0) {
      step_y = -1;
      delta_y = -delta_y;
    }

    // Draw line.
    for (uint32_t i = 0; i < delta_x + 1; i++) {
      draw_pixel(start_x, start_y, outline_colour);

      error += delta_y;

      if (error > delta_x) {
        error -= delta_x;
        start_y += step_y;
      }

      start_x += step_x;
    }
  }

  void draw_rectangle_from(uint32_t* buffer, uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length) {
    // Fill rectangle.
    for (uint32_t i = 0; i < height_length; i++) {
      for (uint32_t j = 0; j < width_length; j++) {
        draw_pixel(x + j, y + i, buffer[(j * i) + j]);
      }
    }
  }

  void draw_rectangle_no_border(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length) {
    for (uint32_t i = 0; i < height_length; i++) {
      for (uint32_t j = 0; j < width_length; j++) {
        draw_pixel(x + j, y + i, fill_colour);
      }
    }
  }
  
  void draw_rectangle(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill) {
    // Draw horizontal lines.
    for (uint32_t i = 0; i < width_length; i++) {
      draw_pixel(x + i, y, outline_colour);
      draw_pixel(x + i, y + height_length - 1, outline_colour);
    }
    
    // Draw vertical lines.
    for (uint32_t i = 0; i < height_length; i++) {
      draw_pixel(x, y + i, outline_colour);
      draw_pixel(x + width_length - 1, y + i, outline_colour);
    }
    
    // Fill rectangle.
    if (fill) {
      for (uint32_t i = 1; i < height_length - 1; i++) {
        for (uint32_t j = 1; j < width_length - 1; j++) {
          draw_pixel(x + j, y + i, fill_colour);
        }
      }
    }
  }

  void shift_screen_horizontal(uint32_t pixels) {
    uint32_t dest_y = 0;
    uint32_t src_y = pixels * graphics::resolution_width;

    for (int i = 0; i < graphics::resolution_height - pixels; i++) {
      for (int j = 0; j < graphics::resolution_width; j++) {
        graphics::draw_pixel_linear(dest_y + j, graphics::pixel_colour_linear(src_y + j));
      }

      src_y += graphics::resolution_width;
      dest_y += graphics::resolution_width;
    }
  }
}
