global LoadIDT
LoadIDT:
    ; Disable PIC temporarily since we haven't configured it yet.
    mov al, 0xff
    out 0xa1, al
    out 0x21, al

    mov edx, dword [esp+4]
    lidt [edx]
    sti

    ret

global TestInt
TestInt:
    mov eax, 1
    ret