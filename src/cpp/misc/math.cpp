#include "math.h"

#include "stdint.h"

namespace math {
  float __attribute__((naked)) sqrt(float x) {
    asm volatile("flds %0" : : "m"(x));
    asm volatile("fsqrt");
    asm volatile("sub $4, %esp");
    asm volatile("fstps %esp");
    asm volatile("pop %eax");
    asm volatile("ret $4");
  }
  
  float hypot(float a, float b) {
    float as = a*a;
    float bs = b*b;
    
    return sqrt(as + bs);
  }
  
  float sin(float x) {
    asm volatile("flds %0" : : "m"(x));
    asm volatile("fsin");
    asm volatile("sub $4, %esp");
    asm volatile("fstps %esp");
    asm volatile("pop %eax");
    asm volatile("ret $4");
  }
}
