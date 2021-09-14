; used by a process to give up control to the kernel and store the return address
; so that once it's the programs turn to use the CPU again, it will return back
; to where it was previously.
global yield_process
yield_process:
    ; pop returning IP from the stack (hack)
    pop eax
    
    ;jmp 
