#include "allocator.h"
#include <stddef.h>

// Overload heap allocation operators.
void* operator new(size_t size) {
    return heap::malloc<void>(size);
}

void* operator new[](size_t size) {
    return heap::malloc<void>(size);
}

void operator delete(void* ptr, size_t size) {
    heap::free<void>(ptr);
};

MARRecord::MARRecord(uint8_t* location_, int size_, bool reserved_) {
    location = location_;
    size = size_;
    reserved = reserved_;
}

int heap::MARRecordCounter = 0;
unsigned int heap::heap_allocated_bytes = 0;

MARRecord* heap::mar_array = (MARRecord*)0x01000000;
uint8_t* heap::next_alloc_addr = (uint8_t*)0x1200000;