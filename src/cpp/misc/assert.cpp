#include "assert.h"
#include "../panic/panic.h"
#include "../structures/string.h"
#include "../misc/conversions.h"

using namespace structures;

void assert_eq(char* name, uint32_t d1, uint32_t d2) {
    if (d1 != d2) {
        string s = "Assert failed for numeric *";
        s.concat(name).concat("*: expected value ");
        s.concat(conversions::u_int_to_char(d2));
        s.concat(" but got ").concat(conversions::u_int_to_char(d1));

        kernel::panic(s);
        __builtin_unreachable();
    }
}

void assert_eq(char* name, structures::string d1, structures::string d2) {
    if (d1 != d2) {
        string s = "Assert failed for numeric *";
        s.concat(name).concat("*: expected value '");
        s.concat(d2);
        s.concat("' but got '").concat(d1).concat("'");

        kernel::panic(s);
        __builtin_unreachable();
    }
}

void assert_eq(char* name, int d1, int d2) {
    if (d1 != d2) {
        string s = "Assert failed for string *";
        s.concat(name).concat("*: expected value ");
        s.concat(conversions::s_int_to_char(d2));
        s.concat(" but got ").concat(conversions::s_int_to_char(d1));

        kernel::panic(s);
        __builtin_unreachable();
    }
}

void assert_eq(char* name, bool d1, bool d2) {
    if (d1 != d2) {
        string s = "Assert failed for boolean *";
        s.concat(name).concat("*: expected value ");
        if (d2 == true) s.concat("true");
        else s.concat("false");
        s.concat(" but got ");
        if (d1 == true) s.concat("true");
        else s.concat("false");

        kernel::panic(s);
        __builtin_unreachable();
    }
}

void assert_eval(char* name, bool condition) {
    if (!condition) {
        string s = "Eval assert failed for *";
        s.concat(name).concat("*");

        kernel::panic(s);
        __builtin_unreachable();
    }
}