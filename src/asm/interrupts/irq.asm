; Same as general interrupts except for the PIC (IRQs) instead.

; Reference outside handler.
extern HandleIRQInterrupt

section .text

; Macro for easy script copying.
%macro IRQBodyMaster 1
    ; Save registers.
    pushad
    pushfd

    ; Since the handler has fastcall attribute, params are passed via registers.
    mov ecx, %1
    call HandleIRQInterrupt

    ; Send EOI.
    mov al, 0x20
    out 0x20, al

    ; Restore registers.
    popfd
    popad

    iret
%endmacro

%macro IRQBodySlave 1
    ; Save registers.
    pushad
    pushfd

    ; Since the handler has fastcall attribute, params are passed via registers.
    mov ecx, %1
    call HandleIRQInterrupt

    ; Send EOI to master and slave.
    mov al, 0x20
    out 0x20, al
    out 0xA0, al

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
    IRQBodyMaster 34

INTERRUPT_35:
    IRQBodyMaster 35

INTERRUPT_36:
    IRQBodyMaster 36

INTERRUPT_37:
    IRQBodyMaster 37

INTERRUPT_38:
    IRQBodyMaster 38

INTERRUPT_39:
    IRQBodyMaster 39

INTERRUPT_40:
    IRQBodyMaster 40


INTERRUPT_41:
    IRQBodySlave 41

INTERRUPT_42:
    IRQBodySlave 42

INTERRUPT_43:
    IRQBodySlave 43

INTERRUPT_44:
    IRQBodySlave 44

INTERRUPT_45:
    IRQBodySlave 45

INTERRUPT_46:
    IRQBodySlave 46

INTERRUPT_47:
    IRQBodySlave 47

INTERRUPT_48:
    IRQBodySlave 48
