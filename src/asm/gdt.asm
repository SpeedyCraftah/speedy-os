global LoadGDT
LoadGDT:
    ; Load GDT pointer.
    mov edx, dword [esp+4]
    lgdt [edx]

    ; Far jump inside the label to switch to the segment after.
    jmp 0x08:ReloadSegments

ReloadSegments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret