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

  ; Jump to program
  return_to_thread_ring3

  ; Load registers from temporary storage.
  load_general_registers_from_temp

  iret
