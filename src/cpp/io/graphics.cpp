#include "graphics.h"

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

  void draw_text(uint16_t* font, uint32_t x_start, uint32_t y_start, char* str, float scale) {
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
      ).ptr();

      for (int y = 0; y < height * scale; y++) {
          uint32_t y_value = y + y_start + offset_y;
          uint32_t index = (y * (width * scale));

          for (int x = 0; x < width * scale; x++) {
              if (resized_data[index + x] == 1) {
                graphics::draw_pixel(x_start + x + offset_x, y_value, outline_colour);
              } 
          }
      }

      offset_x += 2 + width * scale;

      i++;
    }
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
  
  void draw_rectangle(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill) {
    // Draw horizontal lines.
    for (uint32_t i = 0; i < width_length; i++) {
      draw_pixel(x + i, y, outline_colour);
      draw_pixel(x + i + 1, y + height_length, outline_colour);
    }
    
    // Draw vertical lines.
    for (uint32_t i = 0; i < height_length; i++) {
      draw_pixel(x, y + i + 1, outline_colour);
      draw_pixel(x + width_length, y + i, outline_colour);
    }
    
    // Fill rectangle.
    if (fill) {
      for (uint32_t i = 0; i < height_length - 1; i++) {
        for (uint32_t j = 0; j < width_length - 1; j++) {
          draw_pixel(x + j + 1, y + i + 1, fill_colour);
        }
      }
    }
  }
}
