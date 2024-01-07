#pragma once

namespace kernel {
    void panic(char* error = "An unknown exception has occured.", bool hang = true);
}