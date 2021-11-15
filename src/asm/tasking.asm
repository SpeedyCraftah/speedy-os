; Not sure how to include files in different directories without full path at the moment.
%include "/home/speedy/Code/speedyosv2/src/asm/taskingutil.asm"

extern TEMP_REGISTERS
extern on_scheduler_timer_interrupt

; Low-level switch to task implementation.
global scheduler_switch_to_task
scheduler_switch_to_task:
    ; Send EOI.
    mov al, 0x20
    out 0x20, al

    ; Load program registers.
    scheduler_load_registers_from_temp

    ; Load stack.
    mov esp, [TEMP_REGISTERS+16]

    ; Jump to code (EIP).
    jmp [TEMP_REGISTERS+32]

; Halts the CPU upon no tasks being being available to process.
global scheduler_sleep
scheduler_sleep:
    ; Send EOI.
    mov al, 0x20
    out 0x20, al

    ; In case another interrupt wakes the script up other than PIT.
    ; Mostly occurs in slow-timer/hardware environments.
    .loop:
        hlt

        ; Go sleep again.
        jmp .loop