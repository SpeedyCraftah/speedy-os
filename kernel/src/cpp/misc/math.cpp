#include "math.h"

#include "stdint.h"

namespace math {
  // Perform a square root using assembly.
  float sqrt(float x) {
    float result;
    asm("fsqrt" : "=t"(result) : "0"(x));
    return result;
  }
  
  float hypot(float a, float b) {
    float as = a*a;
    float bs = b*b;
    
    return sqrt(as + bs);
  }

  float sin(float x) {
    float result;
    asm("fsin" : "=t"(result) : "0"(x));
    return result;
  }

  float cos(float x) {
    float result;
    asm("fcos" : "=t"(result) : "0"(x));
    return result;
  }

  float tan(float x) {
    float result;
    asm("fptan" : "=t"(result) : "0"(x));
    return result;
  }

  int abs(int x) {
    return x < 0 ? -x : x;
  }
}
