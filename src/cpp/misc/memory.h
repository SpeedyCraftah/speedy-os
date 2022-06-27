#pragma once

#include "stddef.h"

extern "C" void* memcpy(void* dstptr, const void* srcptr, size_t size);
extern "C" void* memset(void* ptr, int value, size_t size);
extern "C" int memcmp(const void* ptr1, const void* ptr2, size_t num);