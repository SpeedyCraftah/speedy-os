%include "/mnt/c/Users/Speedy/Desktop/Code/speedyos/src/asm/scheduling/utils.asm"

extern handle_system_call
extern scheduler_sleep
extern virtual_temporary_registers
extern handle_context_switch
extern temporary_eip
extern syscall_esp_backup

; Handles syscall interrupts from programs.
global INTERRUPT_128
INTERRUPT_128:
  ; Dump registers (plus offset).
  save_general_registers_to_temp 12

  ; Save return EIP.
  mov eax, [esp]
  mov [ecx+32], eax

  ; Backup the ESP.
  mov [syscall_esp_backup], esp

  ; Load the kernel stack.
  load_kernel_stack

  ; ECX = System call number.
  ; EDX = Data.
  call handle_system_call

  cmp eax, 0
  jz .normal_return

  ; Halt until scheduler timer.

  ; TODO - improve this performance wise, loading/saving 2 different stack pointers.
  mov esp, [syscall_esp_backup]

  ; Push scheduler switch address and return.
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
    
    mov eax, [virtual_temporary_registers]
    mov eax, [eax+32]
    mov [esp], eax

    ; Load registers and return.
    load_general_registers_from_temp

    ; Subtract ESP to counter offset.
    sub esp, 12

    ; Return from the interrupt.
    iret
    
