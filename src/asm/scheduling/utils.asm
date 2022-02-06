; Common utility functions for tasking.

extern temporary_registers

; Define location for EIP register.
%define temporary_registers_eip temporary_registers+32

; Send EOI signal to PIC, signaling end of IRQ.
%macro send_eoi 0
    mov al, 0x20
    out 0x20, al
%endmacro

%macro save_general_registers_to_temp 1
    mov [temporary_registers], eax
    mov [temporary_registers+4], ecx
    mov [temporary_registers+8], edx
    mov [temporary_registers+12], ebx

    ; Preserve the stack.
    mov eax, esp
    add eax, %1
    mov [temporary_registers+16], eax

    mov [temporary_registers+20], ebp
    mov [temporary_registers+24], esi
    mov [temporary_registers+28], edi

    ; Dump EFLAGS.
    lahf
    mov byte [temporary_registers+36], ah
%endmacro

%macro load_general_registers_from_temp 0
    ; Load EFLAGS.
    mov ah, byte [temporary_registers+36]
    sahf

    mov eax, [temporary_registers]
    mov ecx, [temporary_registers+4]
    mov edx, [temporary_registers+8]
    mov ebx, [temporary_registers+12]
    mov esp, [temporary_registers+16]
    mov ebp, [temporary_registers+20]
    mov esi, [temporary_registers+24]
    mov edi, [temporary_registers+28]
%endmacro

extern kernel_stack

%macro load_kernel_stack 0
    mov esp, kernel_stack
    mov ebp, 0
%endmacro