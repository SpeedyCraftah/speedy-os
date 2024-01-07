#pragma once

#include "stdint.h"

namespace math {
   // Implements a float vector.
   class vector {
      public:
         vector(uint32_t size, float* numbers);
         ~vector();

         // Adds all of the elements by the specified value.
         vector& add(float value);

         // Subtracts all of the elements by the specified value.
         vector& sub(float value);
         
         // Multiplies all of the elements by the specified value.
         vector& mul(float value);

         // Divides all of the elements by the specified value.
         vector& div(float value);

         // Powers the elements by the specified value (only supports float max).
         vector& pow(float exponent);

         // Normalises all of the elements into values which add up to 1.
         vector& normalize();

         // Returns the total sum of the vector.
         float sum();

         // [] operator.
         float& operator[](uint32_t index);

      private:
         float* storage_ptr;
         uint32_t _size;
   };

   // Returns the hypotenuse of the opposite and adjacent.
   float hypot(float a, float b);
   
   // Returns the square root of x.
   float sqrt(float x);

   // Returns the absolute value of x.
   float log(float x);
   
   // Returns the sin of x.
   float sin(float x);
   
   // Returns the cos of x.
   float cos(float x);

   // Returns the tan of x.
   float tan(float x);

   // Returns the power of the base.
   float pow(float base, float exponent);

   // Returns the absolute value of x.
   int abs(int x);
   
   // Returns value of PI.
   const float PI = 3.1415926535897932384626433832795028841971;
}