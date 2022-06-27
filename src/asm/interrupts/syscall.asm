%include "/mnt/c/Users/jacek/Desktop/Code/speedyos/src/asm/scheduling/utils.asm"

extern handle_system_call
extern scheduler_sleep
extern temporary_registers
extern handle_context_switch
extern temporary_eip

; Handles syscall interrupts from programs.
global INTERRUPT_128
INTERRUPT_128:
  ; Dump registers (plus offset).
  save_general_registers_to_temp 12

  ; Save return EIP.
  mov eax, [esp]
  mov [ecx+32], eax

  ; ECX = System call number.
  ; EDX = Data.

  call handle_system_call

  cmp eax, 0
  jz .normal_return

  ; Halt until scheduler timer.

  ; Push scheduler switch address and return.
  mov [esp], dword .far_return
  iret

  .far_return:
    ; Disable interrupts.
    ; cli

    ; Load the kernel stack.
    load_kernel_stack

    ; Jump to C++ code.
    jmp handle_context_switch

  ; Normal return to program.
  .normal_return:
    ; Replace EIP in case it changed.
    
    mov eax, [temporary_registers]
    mov eax, [eax+32]
    mov [esp], eax

    ; Load registers and return.
    load_general_registers_from_temp

    ; Subtract ESP to counter offset.
    sub esp, 12

    ; Return from the interrupt.
    iret
    
