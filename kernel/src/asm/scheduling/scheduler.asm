; Assembly side of the scheduler.

%include "utils.asm"

extern virtual_temporary_registers
extern temporary_eip
extern kernel_stack
extern timer_preempt

global scheduler_sleep
scheduler_sleep:
  ; Send EOI.
  send_eoi

  ; Restore kernel stack to original.
  mov esp, [kernel_stack]

  ; Enable interrupts.
  sti

  ; Sleep (loop incase it wakes up prematurely).
  .loop:
    hlt
    jmp .loop

global scheduler_execute
scheduler_execute:
  ; Set to false.
  mov byte [timer_preempt], 0

  ; Send EOI.
  send_eoi


  ; Create ring 3 interrupt return;
  ; Load ring 3 segments.
  mov ax, (4 * 8) | 3
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push (4 * 8) | 3 ;ds
  
  ; load registers
  mov ecx, [virtual_temporary_registers]

  ; push stack
  mov eax, [ecx+16]
  push eax ;esp
  
  ;eflags
  mov eax, [ecx+36]
  push eax
  push (3 * 8) | 3 ;cs
  
  mov eax, [temporary_eip]
  push eax ;eip


  ; Load all general registers.
  frstor [ecx+40]
  mov edi, [ecx+28]
  mov esi, [ecx+24]
  mov ebp, [ecx+20]
  mov ebx, [ecx+12]
  mov edx, [ecx+8]
  mov eax, [ecx+0]
  mov ecx, [ecx+4]

  iret