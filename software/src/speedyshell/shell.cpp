#include "shell.h"
#include "main.h"
#include "../../include/sys.h"
#include "../../../shared/graphics/fonts/internal.h"
#include "../../../shared/graphics/graphics.h"

struct OutputFragment {
    uint32_t colour;
    char text[];
};

void output_thread(void* c) {
    int bytes_available;
    while (true) {
        bytes_available = speedyos::fetch_fragment_size_steady_datasink(output_datasink_id);
        if (bytes_available == -1) {
            speedyos::panic("Data sink fetch failed!");
        }
        
        if (bytes_available == 0) {
            speedyos::suspend_thread(100);
            continue;
        }

        // Read the fragment and print to screen.
        // TODO - switch to preallocated buffer in heap.
        uint8_t buffer[bytes_available];
        speedyos::read_fragment_steady_datasink(output_datasink_id, buffer);

        OutputFragment* fragment = (OutputFragment*)buffer;
        print(fragment->text, fragment->colour);
    }
}

void print_prefix() {
    graphics::fill_colour = rgb_colour(34, 221, 34);
    x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, "<user@speedyshell>");
    graphics::fill_colour = rgb_colour(255, 255, 255);
    x_offset += graphics::draw_text(internal_fonts::bios_port_improved, x_offset, y_offset, "$ ");
}