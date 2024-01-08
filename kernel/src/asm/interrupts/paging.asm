%include "utils.asm"

extern HandlePageFault
extern handle_system_call
extern scheduler_sleep
extern virtual_temporary_registers
extern handle_context_switch
extern temporary_eip

global INTERRUPT_14
INTERRUPT_14:
    ; Dump registers (plus offset).
    save_general_registers_to_temp 16

    ; Save return EIP.
    mov eax, [esp+4]
    mov [ecx+32], eax

    mov eax, cr2
    push eax
    
    call HandlePageFault
    add esp, 8

    cmp eax, 0
    jz .normal_return

    ; Halt until scheduler timer.

    ; Push scheduler switch address and return.
    mov [esp], dword .far_return
    iret

    .far_return:
        ; Disable interrupts.
        ; cli

        ; Load the kernel stack.
        load_kernel_stack

        ; Jump to C++ code.
        jmp handle_context_switch

    ; Normal return to program.
    .normal_return:
        ; Replace EIP in case it changed.
        
        mov eax, [virtual_temporary_registers]
        mov eax, [eax+32]
        mov [esp], eax

        ; Load registers and return.
        load_general_registers_from_temp

        ; Subtract ESP to counter offset.
        sub esp, 12

        ; Return from the interrupt.
        iret