global enable_paging_asm
enable_paging_asm:
    mov cr3, ecx

    ; enable global pages
    mov eax, cr4
    or eax, 1<<7
    
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    ret

global disable_paging_asm
disable_paging_asm:
    