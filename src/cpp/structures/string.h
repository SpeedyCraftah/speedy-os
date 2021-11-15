#pragma once

#include "../misc/smart_ptr.h"

namespace structures {
    class string {
        public:
            string(char character);
            string(char* str = nullptr);
            ~string();

            unsigned int length();

            // Dangerous. Kept for performance reasons. Trusts not to be modified at all and only read.
            char* char_reference();

            // Copies the string into another memory location and returns it.
            smart_ptr<char> char_copy();

            // Concats a char* to string.
            string& concat(char* src);

            // Retrieve the hash.
            unsigned int get_weak_hash();
            
            // Assertion of equalness.
            bool hash_equal_to(char* str);
            bool hash_equal_to(string& str);

            // Overload [] operator.
            char& operator[](int index);

            // Overload + operator (strings).
            string operator+(string& src);

            // Overload + operator.
            string operator+(char* src);

            // Overload cast operator.
            operator char*();
        
        private:
            char* storage_ptr = nullptr;

            void _update_hash();

            unsigned int _hash = 0;
            unsigned int _length = 0;
    };
}