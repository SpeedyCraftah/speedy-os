// NOTE TO SELF
// LOADER USES OLD GNU AT&T ASM FORMAT
// FOR COMPATIBILITY REASONS.

// Contains critical instructions required to
// jump the OS from low level code to high level code (c++).

.space 65

// START OF GRUB MULTIBOOT

.set MAGIC, 0x1badb002

// Align OS to 4KB boundaries | Include memory map (TBU) | Set graphics mode.
.set FLAGS, (1<<0 | 1<<1 | 1<<2)

.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 800
    .long 600
    .long 32

// END OF GRUB MULTIBOOT

.section .text
.extern kernelMain
.extern structure_address
.extern callConstructors
.global loader

.global _start
_start:
    movl %ebx, (structure_address)
    movl $kernel_stack, (kernel_stack)
    movl $kernel_stack, %esp
    
    push %eax
    push %ebx
    
    jmp kernelMain

// Stack grows down.
.section .bss
.global kernel_stack
.lcomm kernel_stack, 4

// 4KB kernel stack.
.space 4 * 1024
_kernel_stack:
