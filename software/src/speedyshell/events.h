#pragma once

#include "stdint.h"

void on_key_press(uint32_t id, uint32_t data);
void on_process_end(uint32_t id, uint32_t pid);
void on_modifier_press(uint32_t id, uint32_t data);