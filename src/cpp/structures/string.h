#pragma once

#include "../misc/smart_ptr.h"

namespace structures {
    class string {
        public:
            string(char* str = nullptr);
            ~string();

            unsigned int length();

            // Dangerous. Kept for performance reasons. Trusts not to be modified at all and only read.
            char* char_reference();

            // Copies the string into another memory location and returns it.
            smart_ptr<char> char_copy();

            // Concats a char* to string.
            string& concat(char* src);

            // Overload = operator.
            string& operator=(const string& src);

            // Overload [] operator.
            char operator[](int index);

            // Overload + operator (strings).
            string operator+(string& src);

            // Overload + operator.
            string operator+(char* src);

            // Overload cast operator.
            operator char*();
        
        private:
            char* storage_ptr = nullptr;

            unsigned int _length = 0;
    };
}