; Same as general interrupts except for the PIC (IRQs) instead.

; Reference outside handler.
extern HandleIRQInterrupt

section .text

; Macro for easy script copying.
%macro IRQBody 2
    ; Disable interrupts.
    cli

    ; Save registers.
    pushad
    pushfd

    ; Since the handler has fastcall attribute, params are passed via registers.
    mov ecx, %1
    call HandleIRQInterrupt

    ; Send EOI.
    mov al, 0x20
    out 0x20, al

    %if %2 == 1
        out 0xA0, al
    %endif

    ; Restore registers.
    popfd
    popad

    iret
%endmacro

; IRQs 0-7 (master) 8-15 (slave).
; (IRQs have to be divisble by 8 for some reason).

; Master.
global INTERRUPT_34
global INTERRUPT_35
global INTERRUPT_36
global INTERRUPT_37
global INTERRUPT_38
global INTERRUPT_39
global INTERRUPT_40

; Slave.
global INTERRUPT_41
global INTERRUPT_42
global INTERRUPT_43
global INTERRUPT_44
global INTERRUPT_45
global INTERRUPT_46
global INTERRUPT_47
global INTERRUPT_48

; Define.

; Timer interrupt.
; Directly hooked and managed by scheduler for performance and flexibility reasons.
; Moved to irqtimer.asm.
; INTERRUPT_33:

INTERRUPT_34:
    IRQBody 34, 1

INTERRUPT_35:
    IRQBody 35, 1

INTERRUPT_36:
    IRQBody 36, 1

INTERRUPT_37:
    IRQBody 37, 1

INTERRUPT_38:
    IRQBody 38, 1

INTERRUPT_39:
    IRQBody 39, 1

INTERRUPT_40:
    IRQBody 40, 1


INTERRUPT_41:
    IRQBody 41, 0

INTERRUPT_42:
    IRQBody 42, 0

INTERRUPT_43:
    IRQBody 43, 0

INTERRUPT_44:
    IRQBody 44, 0

INTERRUPT_45:
    IRQBody 45, 0

INTERRUPT_46:
    IRQBody 46, 0

INTERRUPT_47:
    IRQBody 47, 0

INTERRUPT_48:
    IRQBody 48, 0
