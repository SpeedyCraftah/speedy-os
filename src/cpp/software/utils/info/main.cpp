#include "main.h"

#include "../../system/speedyshell/main.h"
#include "../../include/sys.h"

void software::info::start() {
    speedyshell::printf("hello world");

    return speedyos::end_process();
}