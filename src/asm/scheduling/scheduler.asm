; Assembly side of the scheduler.

%include "/home/speedy/Code/speedyosv22/src/asm/scheduling/utils.asm" 

extern temporary_registers

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
  ; Send EOI.
  send_eoi

  ; Load registers from temporary storage.
  load_general_registers_from_temp

  ; Enable interrupts.
  sti

  ; Jump to programs EIP.
  jmp [temporary_registers_eip]
  