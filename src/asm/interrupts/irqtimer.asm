%include "/home/speedy/Code/speedyosv22/src/asm/taskingutil.asm"

extern TEMP_REGISTERS
extern current_process
extern on_scheduler_timer_interrupt_main

; On PIT timer interrupt.
global INTERRUPT_33
INTERRUPT_33:
    ; Save flags & eax register.
    pushfd
    push eax

    ; Check if scheduler is sleeping before saving registers.
    mov eax, [current_process]

    ; 0 = previously running as scheduler.
    cmp eax, 0

    ; Restore register.
    pop eax

    ; If kernel is idling.
    je .idling

    ; Else if kernel was previously executing task.

    ; Restore register.
    popfd

    ; Subtract 12 (+ 4 (backup)) from the stack pointer since the temporary interrupt data is still here.
    push eax
    mov eax, esp
    add eax, 16
    mov [TEMP_REGISTERS+16], eax
    pop eax

    ; Dump all registers (to be changed since this is just a simple version).
    scheduler_dump_registers_to_temp

    ; Dump EIP.
    pop eax
    mov [TEMP_REGISTERS+32], eax

    jmp .finish

    ; Skip over saving registers code to save performance.
    .idling:
        add esp, 8

    .finish:

    ; Replace return address with C++ handler address.
    push on_scheduler_timer_interrupt

    ; Return from interrupt to scheduler.
    iret

; Switches stacks after far jumping from interrupt.
on_scheduler_timer_interrupt:
    scheduler_load_kernel_stack

    jmp on_scheduler_timer_interrupt_main