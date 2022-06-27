; Common utility functions for tasking.

extern temporary_registers

; Send EOI signal to PIC, signaling end of IRQ.
%macro send_eoi 0
    mov al, 0x20
    out 0x20, al
%endmacro

; Return - ECX holds pointer to register data.
%macro save_general_registers_to_temp 1
    ; Preserve ECX.
    push ecx

    ; Load registers.
    mov ecx, [temporary_registers]

    mov [ecx], eax
    mov [ecx+8], edx
    mov [ecx+12], ebx

    ; Preserve the stack.
    mov ebx, esp
    add ebx, 4
    add ebx, %1
    mov [ecx+16], ebx

    mov [ecx+20], ebp
    mov [ecx+24], esi
    mov [ecx+28], edi

    ; Dump EFLAGS.
    lahf
    mov byte [ecx+36], ah

    ; Dump FPU.
    fsave [ecx+37]

    ; Save ecx.
    pop eax
    mov [ecx+4], eax
%endmacro

; Return - None.
%macro load_general_registers_from_temp 0
    mov ecx, [temporary_registers]

    ; Load EFLAGS.
    mov ah, byte [ecx+36]
    sahf

    mov eax, [ecx]
    mov edx, [ecx+8]
    mov ebx, [ecx+12]
    mov esp, [ecx+16]
    mov ebp, [ecx+20]
    mov esi, [ecx+24]
    mov edi, [ecx+28]

    ; Restore FPU.
    frstor [ecx+37]

    ; Restore ecx.
    mov ecx, [ecx+4]
%endmacro

extern kernel_stack

%macro load_kernel_stack 0
    mov esp, kernel_stack
    mov ebp, 0
%endmacro