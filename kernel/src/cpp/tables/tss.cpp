#include "tss.h"
#include "gdt.h"

extern "C" void* kernel_stack;
extern "C" void LoadTSS();

TSS DefaultTSS = {0, 0, 0x10};

void tss_setup_default() {
    asm volatile ("" : : "r"(&kernel_stack));
    uint32_t tss_address = reinterpret_cast<uint32_t>(&DefaultTSS);
    uint32_t tss_size = sizeof(DefaultTSS);

    GDTEntry* entry = &DefaultGDT.TSS;

    entry->Base0 = tss_address & 0xFFFF;
    entry->Base1 = (tss_address >> 16) & 0xFF;
    entry->Base2 = (tss_address >> 24) & 0xFF;
    
    entry->Limit0 = tss_size & 0xFFFF;
    entry->Limit1 = (tss_size >> 16) & 0xF;

    DefaultTSS.esp0 = reinterpret_cast<uint32_t>(kernel_stack);

    LoadTSS();
}