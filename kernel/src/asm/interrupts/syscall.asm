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
  ; Save all registers (+stack).
  push eax
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


  ; ECX = System call number.
  ; EDX = Data.
  ; EAX = 1 = No return, 0 = Return.
  call handle_system_call
  
  test eax, eax
  je .thread_return

  .scheduler_return:
    ; Set cs+dx to kernel segments.
    mov [esp+4], (1 * 8) | 0
    mov [esp+16], (2 * 8) | 0

    ; Set kernel stack.
    mov eax, [kernel_stack]
    mov [esp+12], eax

    ; Disable interrupts and set to IOPL=0 in eflags.
    mov [esp+8], 00000000001000000000000000000010b

    iret
  
  .thread_return:
    ; Load all registers (except eflags).
    mov eax, [virtual_temporary_registers]

    ;esp
    mov ecx, [eax+36]
    mov [esp+12], ecx

    ;eip
    mov ecx, [eax+32]
    mov [esp+0], ecx

    frstor [eax+40]
    mov edi, [eax+28]
    mov esi, [eax+24]
    mov ebp, [eax+20]
    mov ebx, [eax+12]
    mov edx, [eax+8]
    mov ecx, [eax+4]
    mov eax, [eax+0]
    
    iret
