%include "/home/speedy/Code/speedyosv22/src/asm/scheduling/utils.asm"

extern temporary_registers
extern handle_timer_tick

global INTERRUPT_33

; On PIT timer yield (context switch).
INTERRUPT_33:
    ; Future optimisation idea: Check if a thread was executing or not before saving.

    ; Save registers to structure with stack offset to counter stack difference.
    save_general_registers_to_temp 12

    ; Save returning program EIP to temporary register.
    mov eax, [esp]
    mov [temporary_registers_eip], eax

    ; Jump to higher level interrupt handler.
    mov [esp], dword .far_return
    iret

    .far_return:
        ; Disable interrupts.
        cli

        ; Load the kernel stack.
        load_kernel_stack

        ; Jump to C++ code.
        jmp handle_timer_tick