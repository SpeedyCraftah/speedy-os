#include "graphics.h"

#include "stdint.h"
#include "vga_colours.h"

namespace graphics {
  inline void __attribute__((always_inline)) draw_pixel(uint32_t x, uint32_t y, VGA_COLOUR colour) {
    unsigned char* location = (unsigned char*)0xA0000 + 320 * y + x;
    *location = VGA_COLOR;
  }
  
  inline void __attribute__((always_inline)) draw_pixel_linear(uint32_t x, VGA_COLOUR colour) {
    unsigned char* location = (unsigned char*)0xA0000 + x;
    *location = VGA_COLOR;
  }
  
  void draw_rectangle(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill) {
    // Draw horizontal lines.
    for (uint32_t i = 0; i < width_length; i++) {
      draw_pixel(x + i, y, outline_colour);
      draw_pixel(x + i, y + height_length, outline_colour);
    }
    
    // Draw vertical lines.
    for (uint32_t i = 0; i < height_length; i++) {
      draw_pixel(x, y + i, outline_colour);
      draw_pixel(x + width_length, y + i, outline_colour);
    }
    
    // Fill rectangle.
    if (fill) {
      for (uint32_t i = 0; i < height_length * width_length; i++) {
        draw_pixel((i % width_length) + width_length, (uint32_t)(i / height_length), fill_colour); 
      }
    }
  }
}
