#pragma once

#include "stddef.h"

void* memcpy(void* dstptr, const void* srcptr, size_t size);
void* memset(void* ptr, int value, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t num);