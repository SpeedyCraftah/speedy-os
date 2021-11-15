%include "/home/speedy/Code/speedyosv2/src/asm/taskingutil.asm"

extern on_system_call
extern scheduler_sleep
extern TEMP_REGISTERS

; Handles syscall interrupts from programs.
global INTERRUPT_128
INTERRUPT_128:
  ; Dump registers (also save).
  scheduler_dump_registers_to_temp

  ; Save return EIP.
  mov eax, dword [esp]
  mov [TEMP_REGISTERS+32], eax

  ; Save stack.
  ; Add 12 to reach original stack state.
  mov eax, esp
  add eax, 12
  mov [TEMP_REGISTERS+16], eax

  ; ECX = System call number.
  ; EDX = Data.

  call on_system_call

  cmp eax, 0
  je .normal_return

  ; Halt until scheduler timer.

  mov [esp], dword scheduler_sleep

  iret

  ; Normal return to program.
  .normal_return:
    scheduler_load_registers_from_temp

    iret