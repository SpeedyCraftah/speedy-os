// Handles multitasking for processes only (not threads).

#include "manager.h"
#include "structures/process.h"
#include "../../software/system/system.cpp"
#include "../cpu/registers.h"
#include <stdint.h>

unsigned int multitasking::process_manager::next_process_id = 0;
int multitasking::process_manager::current_process_id = 0;

structures::map<int, Process> multitasking::process_manager::processes = structures::map<int, Process>(20);

void multitasking::process_manager::start_process(char* name, Software::ProcessRuntime* processRuntime, TaskPriority priority, ProcessFlag flags) {
    Process process;
    process.priority = priority;
    process.flags = flags;
    process.process = processRuntime;
    process.name = name;
    process.id = next_process_id;

    next_process_id++;

    processes.set(process.id, process);
}

// Starts all multi-tasking processing services.
// Complete control of the CPUs has now been transferred to the tasking manager.
// The kernel process will be automatically started so that it can start other critical/non-critical services
// as well as, well manage the system.
static void multitasking::start_service() {
    
}

// ASM functions.
extern "C" void multitasking::asm_handlers::on_process_yield() {
    uint32_t return_address;
    cpu::registers::eax::value(return_address);

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;

    cpu::registers::eax::value(eax);
    cpu::registers::ecx::value(ecx);
    cpu::registers::edx::value(edx);
    cpu::registers::ebx::value(ebx);
    cpu::registers::esp::value(esp);
    cpu::registers::ebp::value(ebp);
    cpu::registers::esi::value(esi);
    cpu::registers::edi::value(edi);
    // cpu::registers::eflags::value(eflags);

    Process process = process_manager::processes.fetch(process_manager::current_process_id);

    process.registers.eax = eax;
    process.registers.ecx = ecx;
    process.registers.edx = edx;
    process.registers.ebx = ebx;
    process.registers.esp = esp;
    process.registers.ebp = ebp;
    process.registers.esi = esi;
    process.registers.edi = edi;
    process.registers.eflags = eflags;

    process.registers.eip = return_address;

    
}
