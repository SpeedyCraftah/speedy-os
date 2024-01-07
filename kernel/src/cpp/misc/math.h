#pragma once

#include "stdint.h"

namespace math {
   // Returns the hypotenuse of the opposite and adjacent.
   float hypot(float a, float b);
   
   // Returns the square root of x.
   float sqrt(float x);
   
   // Returns the sin of x.
   float sin(float x);
   
   // Returns the cos of x.
   float cos(float x);

   // Returns the tan of x.
   float tan(float x);

   // Returns the absolute value of x.
   int abs(int x);
   
   // Returns value of PI.
   const float PI = 3.1415926535897932384626433832795028841971;
}
