#include "./string.h"
#include "../heap/allocator.h"
#include "../misc/smart_ptr.h"
#include "../io/video.h"
#include "../misc/str.h"
#include "../panic/panic.h"
#include "../misc/algorithm.h"
#include "../misc/conversions.h"

structures::string::string(char character) {
    _length = 2;
    storage_ptr = (char*)heap::malloc(2);

    storage_ptr[0] = character;
    storage_ptr[1] = '\0';

    _update_hash();
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
    storage_ptr = (char*)heap::malloc(i);

    i = 0;

    while (1) {
        storage_ptr[i] = str[i];
        if (str[i] == '\0') break;

        i++;
    }

    _update_hash();
}

structures::string::~string() {
    heap::free(storage_ptr);
}

unsigned int structures::string::length() {
    return _length;
}

unsigned int structures::string::get_weak_hash() {
    return _hash;
}

structures::string& structures::string::concat(char* src) {
    unsigned int srcLength = 0;

    // Terminator is ignored in length.
    while (1) {
        if (src[srcLength] == '\0') break;
        srcLength++;
    }

    char* new_storage_ptr = (char*)heap::malloc(srcLength + _length);

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

    // Update hash.
    _update_hash();

    return *this;
}

// Overload [] operator.
char& structures::string::operator[](int index) {
    if (storage_ptr == nullptr) kernel::panic("Overload operation attempted on an uninitialised string.");
    return storage_ptr[index];
}

// Casting enabled for compatibility with chars.
// Overload cast operator.
structures::string::operator char*() {
    if (storage_ptr == nullptr) kernel::panic("Overload operation attempted on an uninitialised string.");
    return char_reference();
}

// Check if the string starts with paremeter string.
bool structures::string::starts_with(char* str) {
    bool pass = false;

    uint32_t i = 0;

    while (true) {
        char src_char = str[i];
        if (src_char == '\0') break;

        if (storage_ptr[i] == src_char) pass = true;
        else {
            pass = false;
            break;
        }

        i++;
    }

    return pass;
}

structures::flexible_array<char*> structures::string::split_by(char delim) {
    auto array = flexible_array<char*>(10, true);

    uint32_t i = 0;
    uint32_t prev_next = 0;

    while (true) {
        char c = storage_ptr[i];
        if (c == '\0') {
            uint32_t length = i - prev_next;
            char* ptr = new char[length + 1];

            for (uint32_t j = 0; j < length; j++) {
                ptr[j] = storage_ptr[prev_next + j];
            }

            // Add deliminater.
            ptr[length] = '\0';

            // Push to array.
            array.push(ptr);

            break;
        }

        if (c == delim) {
            uint32_t length = i - prev_next;
            char* ptr = new char[length + 1];

            for (uint32_t j = 0; j < length; j++) {
                ptr[j] = storage_ptr[prev_next + j];
            }

            // Add deliminater.
            ptr[length] = '\0';

            // Push to array.
            array.push(ptr);

            // Set previous next.
            prev_next = i + 1;
        }

        i++;
    }

    return array;
}

// Check if the strings are equal via hash.
bool structures::string::hash_equal_to(structures::string& str) {
    return str.get_weak_hash() == _hash;
}

// Character version of above.
bool structures::string::hash_equal_to(char* str) {
    uint32_t char_hash = algorithm::hash_string_fnv1a(str);
    return char_hash == _hash;
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

    char* new_storage_ptr = (char*)heap::malloc(srcLength + _length);

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

    auto str = smart_ptr<char>((char*)heap::malloc(_length));

    int i = 0;

    while (1) {
        str[i] = storage_ptr[i];
        if (str[i] == '\0') break;

        i++;
    }

    return str;
}

void structures::string::_update_hash() {
    uint32_t new_hash = algorithm::hash_string_fnv1a(storage_ptr);
    _hash = new_hash;
}