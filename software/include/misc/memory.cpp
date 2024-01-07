#include "memory.h"

void* memcpy(void *dstptr, const void *srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

void* memset(void* ptr, int value, size_t size) {
	unsigned char val = value;
	unsigned char* dst = (unsigned char*)ptr;
	for (size_t i = 0; i < size; i++) {
		dst[i] = val;
	}
	return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size) {
	unsigned char* p1 = (unsigned char*)ptr1;
	unsigned char* p2 = (unsigned char*)ptr2;
	for (size_t i = 0; i < size; i++) {
		if (p1[i] == p2[i]) continue;
		if (p1[i] < p2[i]) return -1;
		else return 1;
	}
	return 0;
}