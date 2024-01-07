#pragma once

#include <stdint.h>

namespace cpu {
    namespace registers {
        namespace eax {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%eax, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%eax" : : "r" (val));
            }
        };

        namespace ecx {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%ecx, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%ecx" : : "r" (val));
            }
        };

        namespace edx {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%edx, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%edx" : : "r" (val));
            }
        };

        namespace ebx {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%ebx, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%ebx" : : "r" (val));
            }
        };

        namespace esp {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%esp, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%esp" : : "r" (val));
            }
        };

        namespace ebp {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%ebp, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%ebp" : : "r" (val));
            }
        };

        namespace esi {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%esi, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%esi" : : "r" (val));
            }
        };

        namespace edi {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%edi, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%edi" : : "r" (val));
            }
        };

        namespace eflags {
            inline __attribute__((always_inline)) void value(uint32_t &dest) {
                asm volatile("mov %%eflags, %0" : "=r" (dest));
                return;
            };

            inline __attribute__((always_inline)) void set(uint32_t val) {
                asm volatile("mov %0, %%eflags" : : "r" (val));
            }
        };
    }
}
