#include "main.h"

#include "../system/speedyshell/main.h"
#include "../include/sys.h"
#include "../../structures/string.h"
#include "../../misc/conversions.h"

using namespace structures;

void software::random::start() {
    string input = speedyos::speedyshell::fetch_input();
    auto args = input.split_by(' ');

    if (args.get_size() <= 2) {
        speedyos::speedyshell::printf("Please enter two numbers (min, max).");
        return speedyos::end_process();
    }
    
    if (!conversions::char_valid_s_int(args[1]) || !conversions::char_valid_s_int(args[2])) {
        speedyos::speedyshell::printf("One or more parameters are not numbers.");
        return speedyos::end_process();
    }

    uint32_t min = conversions::char_to_s_int(args[1]);
    uint32_t max = conversions::char_to_s_int(args[2]);

    if (min > max) {
        speedyos::speedyshell::printf("The minimum number cannot be larger than the maximum.");
        return speedyos::end_process();
    }

    uint32_t result = min + speedyos::hardware_random() % (( max + 1 ) - min);

    string s = "The generated number is: ";
    s.concat(conversions::s_int_to_char(result))
     .concat(".");

    speedyos::speedyshell::printf(s);

    speedyos::end_process();
}