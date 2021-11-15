; Common utility functions for tasking.

extern program_stack

%macro scheduler_dump_registers_to_temp 0
    mov [TEMP_REGISTERS], eax
    mov [TEMP_REGISTERS+4], ecx
    mov [TEMP_REGISTERS+8], edx
    mov [TEMP_REGISTERS+12], ebx
    
    ; mov [TEMP_REGISTERS+16], esp
    mov [TEMP_REGISTERS+20], ebp
    
    mov [TEMP_REGISTERS+24], esi
    mov [TEMP_REGISTERS+28], edi

    ; Dump EFLAGS.
    lahf
    mov byte [TEMP_REGISTERS+36], ah
%endmacro

%macro scheduler_load_registers_from_temp 0
    ; Load EFLAGS.
    mov ah, byte [TEMP_REGISTERS+36]
    sahf

    mov eax, [TEMP_REGISTERS]
    mov ecx, [TEMP_REGISTERS+4]
    mov edx, [TEMP_REGISTERS+8]
    mov ebx, [TEMP_REGISTERS+12]
    
    ; mov esp, [TEMP_REGISTERS+16]
    mov ebp, [TEMP_REGISTERS+20]
    
    mov esi, [TEMP_REGISTERS+24]
    mov edi, [TEMP_REGISTERS+28]
%endmacro

extern kernel_stack

%macro scheduler_load_kernel_stack 0
    mov esp, kernel_stack
    mov ebp, 0
%endmacro