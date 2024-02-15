#pragma once

#include "stdint.h"
#include "../scheduling/structures/process.h"

namespace loader {
    Process* load_elf32_executable_as_process(char* name, ProcessFlags flags, void* elf_start, uint32_t elf_size);
}