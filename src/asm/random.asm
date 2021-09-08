; allows code to check whether the intel random instruction is available
; and provides use to it.

global rdrand_available
rdrand_available:
    mov eax, 1  
    mov ecx, 0
    cpuid
    shr ecx, 30
    and ecx, 1
    mov eax, ecx
    ret