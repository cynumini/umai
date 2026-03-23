#ifndef MATH_H
#define MATH_H

#include "sakana.h"

f32 f32_max(f32 a, f32 b);
f32 f32_min(f32 a, f32 b);
f32 f32_clamp(f32 value, f32 min, f32 max);

i32 i32_max(i32 a, i32 b);
i32 i32_min(i32 a, i32 b);
i32 i32_clamp(i32 value, i32 min, i32 max);

#endif // MATH_H