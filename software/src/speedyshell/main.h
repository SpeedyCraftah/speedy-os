#pragma once

#include "stdint.h"

#define MAX_TEXT_BUFFER_SIZE 100

extern uint32_t running_pid;
extern bool caps_text;
extern bool input_allowed;
extern char text_buffer[100];
extern uint32_t text_buffer_ptr;

extern uint32_t y_offset;
extern uint32_t x_offset;

extern uint32_t cursor_x;
extern uint32_t cursor_y;

void cursor_move(uint32_t new_x, uint32_t new_y);