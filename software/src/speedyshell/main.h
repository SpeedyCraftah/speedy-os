#pragma once

#include "stdint.h"
#include "../../include/mutex.h"

#define MAX_TEXT_BUFFER_SIZE 69

extern uint32_t running_pid;
extern bool caps_text;
extern bool input_allowed;
extern char text_buffer[MAX_TEXT_BUFFER_SIZE];
extern uint32_t text_buffer_ptr;

extern uint32_t max_char_height;
extern uint32_t y_offset;
extern uint32_t x_offset;

extern uint32_t cursor_x;
extern uint32_t cursor_y;
extern uint32_t cursor_thread_id;

extern ThreadMutex cursor_mutex;

void cursor_remove();
void cursor_move(uint32_t new_x, uint32_t new_y, bool remove_old = true);