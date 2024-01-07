; Does the dirty work to call a single C++ function for multiple interrupts.

; Reference outside handler.
extern HandleGeneralCPUException

; Macro for easy script copying.
%macro ISRBody 1
    ; Save registers.
    pushad
    pushfd

    ; Since the handler has fastcall attribute, params are passed via registers.
    mov ecx, %1
    call HandleGeneralCPUException

    ; Restore registers.
    popfd
    popad

    iret
%endmacro

; CPU built-in 32 interrupts.

global INTERRUPT_0
global INTERRUPT_1
; Non-maskable interrupt.
; global INTERRUPT_2
global INTERRUPT_3
global INTERRUPT_4
global INTERRUPT_5
global INTERRUPT_6
global INTERRUPT_7
global INTERRUPT_8
; External FPU error. (external FPUs are older than most people reading this)
; global INTERRUPT_9
global INTERRUPT_10
global INTERRUPT_11
global INTERRUPT_12
global INTERRUPT_13
global INTERRUPT_14
; Reserved.
; global INTERRUPT_15
global INTERRUPT_16
global INTERRUPT_17
global INTERRUPT_18
global INTERRUPT_19
global INTERRUPT_20
; Start of reserved sequential.
; global INTERRUPT_21
; global INTERRUPT_22
; global INTERRUPT_23
; global INTERRUPT_24
; global INTERRUPT_25
; global INTERRUPT_26
; global INTERRUPT_27
; global INTERRUPT_28
; global INTERRUPT_29
; End of reserved sequential.
global INTERRUPT_30
; Reserved.
; global INTERRUPT_31

; Implement. (there 100% is a better way to do this.)
INTERRUPT_0:
    ISRBody 0

INTERRUPT_1:
    ISRBody 1

INTERRUPT_2:
    ISRBody 2

INTERRUPT_3:
    ISRBody 3

INTERRUPT_4:
    ISRBody 4

INTERRUPT_5:
    ISRBody 5

INTERRUPT_6:
    ISRBody 6

INTERRUPT_7:
    ISRBody 7

INTERRUPT_8:
    ISRBody 8

INTERRUPT_10:
    ISRBody 10

INTERRUPT_11:
    ISRBody 11

INTERRUPT_12:
    ISRBody 12

INTERRUPT_13:
    ISRBody 13

; removed in favour of paging handler
;INTERRUPT_14:
;    ISRBody 14

INTERRUPT_16:
    ISRBody 16

INTERRUPT_17:
    ISRBody 17

INTERRUPT_18:
    ISRBody 18

INTERRUPT_19:
    ISRBody 19

INTERRUPT_20:
    ISRBody 20

INTERRUPT_30:
    ISRBody 30
