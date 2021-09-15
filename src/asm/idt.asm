global LoadIDT
LoadIDT:
    mov edx, dword [esp+4]
    lidt [edx]
    sti

    ret

EXTERN HandleInterrupt

global OnInterrupt
OnInterrupt:
    call HandleInterrupt
    ret