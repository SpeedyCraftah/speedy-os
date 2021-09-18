#include "./string.h"
#include "../heap/allocator.h"
#include "../misc/smart_ptr.h"
#include "../io/video.h"
#include "../misc/str.h"
#include "../panic/panic.h"

structures::string::string(char character) {
    _length = 2;
    storage_ptr = heap::malloc<char>(2);

    storage_ptr[0] = character;
    storage_ptr[1] = '\0';
}

structures::string::string(char* str) {
    if (str == nullptr) return;

    // Determine char length.
    int i = 0;

    while (1) {
        if (str[i++] == '\0') break;
    }

    // Length + terminator.
    _length = i;
    storage_ptr = heap::malloc<char>(i);

    i = 0;

    while (1) {
        storage_ptr[i] = str[i];
        if (str[i] == '\0') break;

        i++;
    }
}

structures::string::~string() {
    heap::free(storage_ptr);
}

unsigned int structures::string::length() {
    return _length;
}

structures::string& structures::string::concat(char* src) {
    unsigned int srcLength = 0;

    // Terminator is ignored in length.
    while (1) {
        if (src[srcLength] == '\0') break;
        srcLength++;
    }

    char* new_storage_ptr = heap::malloc<char>(srcLength + _length);

    int previous = 0;
    int i = 0;

    if (storage_ptr != nullptr) {
        // Copy original.
        while (1) {
            // Do not copy over terminator.
            if (storage_ptr[i] == '\0') break;
            new_storage_ptr[i] = storage_ptr[i];

            previous++;
            i++;
        }

        i = 0;
    }

    // Copy over source char.
    while (1) {
        new_storage_ptr[previous + i] = src[i];
        if (src[i] == '\0') break;

        i++;
    }

    // Deallocate old storage pointer.
    if (storage_ptr != nullptr) heap::free(storage_ptr);

    // Assign new storage pointer.
    storage_ptr = new_storage_ptr;
    
    // Update length.
    _length = srcLength + _length;

    return *this;
}

// Overload [] operator.
char structures::string::operator[](int index) {
    if (storage_ptr == nullptr) kernel::panic("Overload operation attempted on an uninitialised string.");
    return storage_ptr[index];
}

// Casting enabled for compatibility with chars.
// Overload cast operator.
structures::string::operator char*() {
    if (storage_ptr == nullptr) kernel::panic("Overload operation attempted on an uninitialised string.");
    return char_reference();
}

// Overload + operator (strings).
structures::string structures::string::operator+(string& src) {
    return operator+(src.char_reference());
}

// Overload + operator.
structures::string structures::string::operator+(char* src) {
    unsigned int srcLength = 0;

    // Terminator is ignored in length.
    while (1) {
        if (src[srcLength] == '\0') break;
        srcLength++;
    }

    char* new_storage_ptr = heap::malloc<char>(srcLength + _length);

    int previous = 0;
    int i = 0;

    if (storage_ptr != nullptr) {
        // Copy original.
        while (1) {
            // Do not copy over terminator.
            if (storage_ptr[i] == '\0') break;
            new_storage_ptr[i] = storage_ptr[i];

            previous++;
            i++;
        }

        i = 0;
    }

    // Copy over source char.
    while (1) {
        new_storage_ptr[previous + i] = src[i];
        if (src[i] == '\0') break;

        i++;
    }

    // New string.
    string new_string;
    new_string.storage_ptr = new_storage_ptr;
    new_string._length = srcLength + _length;

    return new_string;
}

char* structures::string::char_reference() {
    if (storage_ptr == nullptr) kernel::panic("Reference attempted on an uninitialised string.");
    return storage_ptr;
}

// Returns a smart pointer.
smart_ptr<char> structures::string::char_copy() {
    if (storage_ptr == nullptr) kernel::panic("Copy attempted on an uninitialised string.");

    auto str = smart_ptr<char>(heap::malloc<char>(_length));

    int i = 0;

    while (1) {
        str[i] = storage_ptr[i];
        if (str[i] == '\0') break;

        i++;
    }

    return str;
}