; Used for scheduler as a cookie cutter FPU state for new processes.
extern fpu_boot_state
global save_fpu_boot_state
save_fpu_boot_state:
    fsave [fpu_boot_state]
    ret