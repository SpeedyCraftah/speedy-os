%include "utils.asm"

extern handle_system_call
extern scheduler_sleep
extern virtual_temporary_registers
extern handle_context_switch
extern temporary_eip

extern debug_val

section .bss
debug_val: resd 1

section .text
; Handles syscall interrupts from programs.
global INTERRUPT_128
INTERRUPT_128:
  ; Dump registers.
  save_general_registers_to_temp 0

  ; Save interrupt frame.
  ;save_interrupt_frame

  ; Save return EIP.
  mov eax, [esp]
  mov [ecx+32], eax

  ; Load the kernel stack.
  ;load_kernel_stack

  ; ECX = System call number.
  ; EDX = Data.
  call handle_system_call

  cmp eax, 0
  jz .normal_return

  ; Load the interrupt frame since we switched to kernel stack.
  ;push_interrupt_frame

  ; Push scheduler switch address and return.
  modify_return_to_ring0
  mov [esp], dword .far_return
  iret

  .far_return:
    ; Disable interrupts.
    ; cli

    ; Load the kernel stack.
    ; load_kernel_stack

    ; Jump to C++ code.
    jmp handle_context_switch

  ; Normal return to program.
  .normal_return:
    ; Replace EIP in case it changed.
    
    mov ecx, [virtual_temporary_registers]
    mov eax, [ecx+32]
    mov [esp], eax

    ; Replace ESP in case it changed.
    mov esp, [ecx+16]

    ; Load registers and return.
    load_general_registers_from_temp

    ; Subtract ESP to counter offset.
    ;sub esp, 12

    ; Return from the interrupt.
    iret
    
