; Used to calculate the TSC modifier value for accurate timing.

section .bss
global tsc_start_low
global tsc_start_high
global tsc_end_low
global tsc_end_high

tsc_start_low: resd 0
tsc_start_high: resd 0

tsc_end_low: resd 0
tsc_end_high: resd 0

section .text
handle_pit_tsc_test_tick:
    mov eax, [tsc_end_low]
    cmp eax, 0
    jnz .done

    mov eax, [tsc_start_low]
    cmp eax, 0
    jnz .record_end

    .record_start:
        rdtsc
        mov [tsc_start_low], eax
        mov [tsc_start_high], edx
        iret

    .record_end:
        rdtsc
        mov [tsc_end_low], eax
        mov [tsc_end_high], edx
        iret

    .done:
        ; call some function
        iret