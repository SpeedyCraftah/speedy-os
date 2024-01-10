#include "tss.h"
#include "gdt.h"

extern "C" uint32_t kernel_stack;

TSS DefaultTSS = {0, kernel_stack, 0x10};

void tss_install_default() {
    uint32_t tss_address = reinterpret_cast<uint32_t>(&DefaultTSS);
    uint32_t tss_size = sizeof(DefaultTSS);

    GDTEntry* entry = &DefaultGDT.TSS;

    entry->Base0 = tss_address & 0xFFFF;
    entry->Base1 = (tss_address >> 16) & 0xFF;
    entry->Base2 = (tss_address >> 24) & 0xFF;
    
    entry->Limit0 = tss_size & 0xFFFF;
    entry->Limit1 = (tss_size >> 16) & 0xF;

    entry->AccessByte = 1;
}