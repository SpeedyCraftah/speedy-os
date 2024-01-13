%include "utils.asm"

extern virtual_temporary_registers
extern handle_timer_tick
extern temporary_eip

global INTERRUPT_33

; On PIT timer yield (context switch).
INTERRUPT_33:
    ; Future optimisation idea: Check if a thread was executing or not before saving.

    ; Save all registers (+stack).
    push eax
    mov eax, [virtual_temporary_registers]

    mov [eax+4], ecx
    mov [eax+8], edx
    mov [eax+12], ebx
    mov [eax+20], ebp
    mov [eax+24], esi
    mov [eax+28], edi
    fsave [eax+40]

    ;esp
    mov ecx, [esp+4+12]
    mov [eax+16], ecx

    ;eip
    mov ecx, [esp+4+0]
    mov [eax+32], ecx

    ;eflags
    mov ecx, [esp+4+8]
    mov [eax+36], ecx

    ;eax
    pop ecx
    mov [eax+0], ecx


    ; Set kernel stack.
    mov eax, [kernel_stack]
    mov [esp+12], eax

    ; Disable interrupts and set to IOPL=0 in eflags.
    mov [esp+8], dword 00000000001000000000000000000010b

    ; Load scheduler address.
    mov [esp+0], handle_timer_tick

    iret