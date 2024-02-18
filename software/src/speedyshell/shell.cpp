#include "shell.h"
#include "main.h"
#include "../../../shared/graphics/fonts/internal.h"
#include "../../../shared/graphics/graphics.h"

void print_prefix() {
    graphics::fill_colour = rgb_colour(34, 221, 34);
    x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, "<user@speedyshell>");
    graphics::fill_colour = rgb_colour(255, 255, 255);
    x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, "$ ");
}