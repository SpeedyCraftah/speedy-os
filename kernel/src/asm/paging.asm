global enable_paging_asm
enable_paging_asm:
    mov cr3, ecx
    
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    ret

global disable_paging_asm
disable_paging_asm:
    