// NOTE TO SELF
// LOADER USES OLD GNU AT&T ASM FORMAT
// FOR COMPATIBILITY REASONS.

// Contains critical instructions required to
// jump the OS from low level code to high level code (c++).

.space 65

// START OF GRUB MULTIBOOT

.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

// END OF GRUB MULTIBOOT

.section .text
.extern kernelMain
.extern callConstructors
.global loader

loader:
    mov $kernel_stack, %esp
    call callConstructors
    push %eax
    push %ebx
    
    // Calling is unnecessary and wastes memory as we'll never return.
    jmp kernelMain

// Stack grows down.
.section .bss 
.space 2 * 1024 * 1024
kernel_stack:
    
