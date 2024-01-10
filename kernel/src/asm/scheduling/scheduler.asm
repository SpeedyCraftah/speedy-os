; Assembly side of the scheduler.

%include "utils.asm"

extern virtual_temporary_registers
extern temporary_eip
extern timer_preempt

global scheduler_sleep
scheduler_sleep:
  ; Send EOI.
  send_eoi

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

  ; Load registers from temporary storage.
  load_general_registers_from_temp

  ; Enable interrupts.
  ; sti

  ; preserve eax
  ; todo - check for stack leak
  push eax

  mov ax, (4 * 8) | 3
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov eax, esp
  push (4 * 8) | 3 ;ds
  push eax ;esp
  pushfd ;eflags
  push (3 * 8) | 3 ;cs
  
  mov eax, [temporary_eip]
  push eax ;eip

  ; restore eax
  mov eax, [esp+20]

  iret
  

  ; Jump to programs EIP.
  ;jmp [temporary_eip]
