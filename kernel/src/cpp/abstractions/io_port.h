#pragma once

#include <stdint.h>
#include "./cpu.h"

namespace io_port {
  namespace bit_8 {
    // I consider myself a little smarter than the compiler on inlining.

    inline __attribute__((always_inline)) void out(uint16_t port, uint8_t data) {
      asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
    }

    inline __attribute__((always_inline)) uint8_t in(uint16_t port) {
      uint8_t data;
      asm volatile("inb %1, %0" : "=a" (data) : "dN" (port));

      return data;
    }

    // Delays a tiny amount of time (usually used to let old PICs to catch up).
    inline __attribute__((always_inline)) void delay_void() {
      // Send data to empty port.
      out(0x80, 0);
    }
  }

  namespace bit_16 {
    inline __attribute__((always_inline)) void out(uint16_t port, uint16_t data) {
      asm volatile("outw %1, %0" : : "dN" (port), "a" (data));
    }

    inline __attribute__((always_inline)) uint16_t in(uint16_t port) {
      uint16_t data;
      asm volatile("inw %1, %0" : "=a" (data) : "dN" (port));

      return data;
    }
  }
}
