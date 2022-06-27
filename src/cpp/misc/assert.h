#pragma once

#include "stdint.h"
#include "../structures/string.h"

void assert_eq(char* name, uint32_t d1, uint32_t d2);
void assert_eq(char* name, structures::string d1, structures::string d2);
void assert_eq(char* name, int d1, int d2);
void assert_eq(char* name, bool d1, bool d2);

void assert_eval(char* name, bool condition);