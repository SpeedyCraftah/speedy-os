; Assembly side of the scheduler.

%include "/mnt/c/Users/Speedy/Desktop/Code/speedyos/src/asm/scheduling/utils.asm"

extern temporary_registers
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
  sti

  ; Jump to programs EIP.
  jmp [temporary_eip]
