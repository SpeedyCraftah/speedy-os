#pragma once

#include "stdint.h"

#define MAX_TEXT_BUFFER_SIZE 100

extern uint32_t running_pid;
extern bool caps_text;
extern bool input_allowed;
extern char text_buffer[100];
extern uint32_t text_buffer_ptr;

