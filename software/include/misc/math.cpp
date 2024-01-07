#include "math.h"

#include "stdint.h"
#include "../alloc.h"

namespace math {
  vector::vector(uint32_t size, float* numbers) {
    _size = size;
    storage_ptr = (float*)malloc(size * sizeof(float));

    // Copy over the elements.
    for (uint32_t i = 0; i < size; i++) {
      storage_ptr[i] = numbers[i];
    }
  }

  vector::~vector() {
    delete storage_ptr;
  }

  vector& vector::add(float value) {
    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] += value;
    }

    return *this;
  }

  vector& vector::sub(float value) {
    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] -= value;
    }

    return *this;
  }

  vector& vector::mul(float value) {
    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] = storage_ptr[i] * value;
    }

    return *this;
  }

  vector& vector::div(float value) {
    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] = (int)(storage_ptr[i] / value);
    }

    return *this;
  }

  vector& vector::pow(float exponent) {
    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] = (int)(math::pow(storage_ptr[i], exponent));
    }

    return *this;
  }

  vector& vector::normalize() {
    float totalSum = sum();

    for (uint32_t i = 0; i < _size; i++) {
      storage_ptr[i] = storage_ptr[i] / totalSum;
    }

    return *this;
  }

  float vector::sum() {
    float sum = 0;

    for (uint32_t i = 0; i < _size; i++) {
      sum += storage_ptr[i];
    }

    return sum;
  }

  float& vector::operator[](uint32_t index) {
    return storage_ptr[index];
  }

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

  float pow(float base, float exponent) {
    float result;
    asm("fld1" : : : "st(1)");
    asm("fldl2t" : : : "st(1)");
    asm("fmulp" : "=t"(result) : "0"(base), "u"(exponent));
    return result;
  }

  // Calculate the logarithm of x.
  float log(float x) {
    float result;
    asm("fldln2" : : : "st(1)");
    asm("fldl2e" : : : "st(1)");
    asm("fmulp" : "=t"(result) : "0"(x), "u"(x));
    asm("fyl2x" : : : "st(1)");
    return result;
  }
}
