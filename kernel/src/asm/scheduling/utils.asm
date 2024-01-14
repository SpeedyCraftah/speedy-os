; Common utility functions for tasking.

extern virtual_temporary_registers

%define KERNEL_EFLAGS dword 00000000001000000000000000000010b

; Send EOI signal to PIC, signaling end of IRQ.
%macro send_eoi 0
    mov al, 0x20
    out 0x20, al
%endmacro

extern kernel_stack

%macro load_kernel_stack 0
    mov esp, [kernel_stack]
    xor ebp, ebp
%endmacro