#include "_shared.h"
#include "string.h"
#include "smart_ptr.h"
#include "str.h"
#include "algorithm.h"
#include "conversions.h"

structures::string::string(char character) {
    _length = 2;
    storage_ptr = (char*)_shared_malloc(2);

    storage_ptr[0] = character;
    storage_ptr[1] = '\0';

    _update_hash();
}

structures::string::string(char* str) {
    // Calculate string length (excl terminator).
    uint32_t length = 0;

    while (1) {
        if (str[length] == '\0') break;
        length++;
    }

    _length = length + 1;

    storage_ptr = new char[_length];

    // Copy over to pointer.
    for (uint32_t i = 0; i < length; i++) {
        storage_ptr[i] = str[i];
    }

    // Add terminator.
    storage_ptr[length] = '\0';

    _update_hash();
}

structures::string::~string() {
    _shared_free(storage_ptr);
}

unsigned int structures::string::length() {
    return _length - 1;
}

unsigned int structures::string::get_weak_hash() {
    return _hash;
}

structures::string& structures::string::concat(char src) {
    // Safety mechanism for null ptrs (will be fixed).
    if (storage_ptr == nullptr) _shared_panic("Attempted to concat a nullptr string.");

    // Get length of strings (excl terminator).
    uint32_t srcLength = 1;
    uint32_t currentLength = _length - 1;

    // Create place in heap (including terminator).
    char* new_storage_ptr = new char[srcLength + currentLength + 1];

    // Copy old string to new string.
    for (uint32_t i = 0; i < currentLength; i++) {
        new_storage_ptr[i] = storage_ptr[i];
    }

    // Copy src to new string.
    new_storage_ptr[currentLength] = src;

    // Add terminator.
    new_storage_ptr[srcLength + currentLength] = '\0';

    // Deallocate old storage pointer.
    if (storage_ptr != nullptr) _shared_free(storage_ptr);

    // Assign new storage pointer.
    storage_ptr = new_storage_ptr;

    // Update length.
    _length = srcLength + currentLength + 1;

    // Update hash.
    _update_hash();

    return *this;
}

structures::string& structures::string::concat(char* src) {
    // Safety mechanism for null ptrs (will be fixed).
    if (storage_ptr == nullptr) _shared_panic("Attempted to concat a nullptr string.");

    // Get length of strings (excl terminator).
    uint32_t srcLength = 0;

    while (1) {
        if (src[srcLength] == '\0') break;
        srcLength++;
    }

    uint32_t currentLength = _length - 1;

    // Create place in heap (including terminator).
    char* new_storage_ptr = new char[srcLength + currentLength + 1];

    // Copy old string to new string.
    for (uint32_t i = 0; i < currentLength; i++) {
        new_storage_ptr[i] = storage_ptr[i];
    }

    // Copy src to new string.
    for (uint32_t i = 0; i < srcLength; i++) {
        new_storage_ptr[currentLength + i] = src[i];
    }

    // Add terminator.
    new_storage_ptr[srcLength + currentLength] = '\0';

    // Deallocate old storage pointer.
    if (storage_ptr != nullptr) _shared_free(storage_ptr);

    // Assign new storage pointer.
    storage_ptr = new_storage_ptr;

    // Update length.
    _length = srcLength + currentLength + 1;

    // Update hash.
    _update_hash();

    return *this;
}

// Overload [] operator.
char& structures::string::operator[](int index) {
    if (storage_ptr == nullptr) _shared_panic("Overload operation attempted on an uninitialised string.");
    return storage_ptr[index];
}

// Casting enabled for compatibility with chars.
// Overload cast operator.
structures::string::operator char*() {
    if (storage_ptr == nullptr) _shared_panic("Overload operation attempted on an uninitialised string.");
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
    auto array = flexible_array<char*>(7, true);
    int prev_delim_i = -1;

    for (uint32_t i = 0; i < _length; i++) {
        char c = storage_ptr[i];

        // If character is the deliminator or terminator.
        if (c == delim || c == '\0') {
            // Push previous index and current index boundary to array.

            uint32_t length = i - (prev_delim_i + 1);
            char* part = new char[length + 1];
            part[length] = 0;

            for (uint32_t j = 0; j < length; j++) {
                part[j] = storage_ptr[prev_delim_i + j + 1];
            }

            // Push to the array.
            array.push(part);

            // Update deliminator location.
            prev_delim_i = i;
        }
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

// Overload == operator.
bool structures::string::operator==(char* src) {
    return hash_equal_to(src);
}

bool structures::string::operator==(const char* src) {
    return hash_equal_to((char*)src);
}

// Overload += operator.
structures::string& structures::string::operator+=(char* src) {
    concat(src);

    return *this;
}

// Overload + operator (strings).
structures::string structures::string::operator+(string& src) {
    return operator+(src.char_reference());
}

// Overload + operator.
structures::string structures::string::operator+(char* src) {
    string new_string = string(storage_ptr);
    new_string.concat(src);

    return new_string;
}

char* structures::string::char_reference() {
    if (storage_ptr == nullptr) _shared_panic("Reference attempted on an uninitialised string.");
    return storage_ptr;
}

// Returns a smart pointer.
smart_ptr<char> structures::string::char_copy() {
    if (storage_ptr == nullptr) _shared_panic("Copy attempted on an uninitialised string.");

    auto str = smart_ptr<char>((char*)_shared_malloc(_length));

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