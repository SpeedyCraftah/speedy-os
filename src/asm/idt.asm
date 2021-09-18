global LoadIDT
LoadIDT:
    mov edx, dword [esp+4]
    lidt [edx]
    sti

    ret

global TestInt
TestInt:
    mov eax, 1
    ret