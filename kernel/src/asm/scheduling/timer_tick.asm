%include "utils.asm"

extern virtual_temporary_registers
extern handle_timer_tick
extern temporary_eip
extern current_thread

global INTERRUPT_33

; On PIT timer yield (context switch).
INTERRUPT_33:
    push eax
    mov eax, [current_thread]
    test eax, eax
    je .scheduler_sleep_finish

    .thread_exec_finish:
        ; Save all registers (+stack).
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

        ; Set cs+dx to kernel segments.
        mov [esp+4], dword (1 * 8) | 0
        mov [esp+16], dword (2 * 8) | 0

        ; Set kernel stack.
        mov eax, [kernel_stack]
        mov [esp+12], eax

        ; Disable interrupts and set to IOPL=0 in eflags.
        mov [esp+8], KERNEL_EFLAGS

        ; Load scheduler address.
        mov [esp+0], dword handle_timer_tick

        iret

    .scheduler_sleep_finish:
        add esp, 4

        ; Load scheduler address.
        mov [esp+0], dword handle_timer_tick

        ; Disable interrupts and set to IOPL=0 in eflags.
        mov [esp+8], KERNEL_EFLAGS

        iret