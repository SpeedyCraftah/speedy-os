#pragma once

#include "stdint.h"
#include "vga_colours.h"

// Graphics manipulation software, inspired partially by HTML canvas.

namespace graphics {
  extern VGA_COLOUR fill_colour = 1;
  extern VGA_COLOUR outline_colour = 1;
  extern uint32_t outline_width = 2;
  
  // Plots a single pixel at the x and y coordinates.
  void draw_pixel(uint32_t x, uint32_t y, VGA_COLOUR colour);
  
  // Plots a single pixel via a 1D linear coordinate.
  void draw_pixel_linear(uint32_t x, VGA_COLOUR colour);
  
  // Draws a rectangle. Outline width will be taken into account in the future.
  // X - Top left corner coordinate. Y - Top left corner Y coordinate. Length - Length in pixels from corner points.
  void draw_rectangle(uint32_t x, uint32_t y, uint32_t width_length, uint32_t height_length, bool fill = false);
  
  // Draws a line, straight or diagonal.
  // Width will be taken into account in the future.
  void draw_line(uint32_t x, uint32_t y);
