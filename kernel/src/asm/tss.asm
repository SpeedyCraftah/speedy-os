global LoadTSS

LoadTSS:
    mov ax, 0x28
    ltr ax
    ret