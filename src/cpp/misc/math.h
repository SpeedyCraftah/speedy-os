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
   
   // Returns value of PI (non-cached).
   float PI();
   
}
