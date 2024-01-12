; Common utility functions for tasking.

extern virtual_temporary_registers

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
    mov ecx, [virtual_temporary_registers]

    mov [ecx], eax
    mov [ecx+8], edx
    mov [ecx+12], ebx

    mov ebx, [esp+8+4+%1]
    mov [ecx+16], ebx

    mov [ecx+20], ebp
    mov [ecx+24], esi
    mov [ecx+28], edi

    ; Dump EFLAGS.
    pushfd
    pop eax
    mov [eax+36], ecx

    ; Dump FPU.
    fsave [ecx+40]

    ; Save ecx.
    pop eax
    mov [ecx+4], eax
%endmacro

; Return - None.
%macro load_general_registers_from_temp 0
    mov ecx, [virtual_temporary_registers]

    ; Load EFLAGS.
    mov eax, [ecx+36]
    push eax
    popfd

    mov eax, [ecx]
    mov edx, [ecx+8]
    mov ebx, [ecx+12]
    ;mov esp, [ecx+16]
    mov ebp, [ecx+20]
    mov esi, [ecx+24]
    mov edi, [ecx+28]

    ; Restore FPU.
    frstor [ecx+40]

    ; Restore ecx.
    mov ecx, [ecx+4]
%endmacro

extern kernel_stack

%macro load_kernel_stack 0
    mov esp, [kernel_stack]
    mov ebp, 0
%endmacro

extern temporary_interrupt_frame

%macro save_interrupt_frame 0
    ; EIP
    mov eax, dword [esp]
    mov dword [temporary_interrupt_frame], eax

    ; CS+padding
    mov eax, dword [esp+4]
    mov dword [temporary_interrupt_frame+4], eax

    ; EFLAGS
    mov eax, dword [esp+8]
    mov dword [temporary_interrupt_frame+8], eax
%endmacro

%macro push_interrupt_frame 0
    ; EFLAGS
    mov eax, dword [temporary_interrupt_frame+8]
    push eax

    ; CS+padding
    mov eax, dword [temporary_interrupt_frame+4]
    push eax

    ; EFLAGS
    mov eax, dword [temporary_interrupt_frame]
    push eax
%endmacro

%macro return_to_thread_ring3 0
    ; preserve eax
    ; todo - check for stack leak
    ;push eax

    mov ax, (2 * 8) | 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;mov eax, esp
    push (2 * 8) | 3 ;ds
    ;push eax ;esp
    
    ; push stack
    mov ecx, [virtual_temporary_registers]
    mov eax, [ecx+16]
    push eax ;esp
    
    pushfd ;eflags
    push (1 * 8) | 0 ;cs
    
    mov eax, [temporary_eip]
    push eax ;eip

    ; restore eax
    mov eax, [esp+20]
%endmacro

%macro modify_return_to_ring0 0
    ; Change EFLAGS to jump to ring 0 with no interrupts.
    mov [esp+8], dword 00000000001000000000000000000010b

    ; Change segment registers to kernel.
    ; try doing without
    mov [esp+4], dword (1 * 8) | 0
    mov [esp+16], dword (2 * 8) | 0

    mov eax, dword [kernel_stack]
    mov [esp+12], dword eax
%endmacro