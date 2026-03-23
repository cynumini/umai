#include "math.h"

f32 f32_max(f32 a, f32 b) { return a > b ? a : b; }
f32 f32_min(f32 a, f32 b) { return a < b ? a : b; }
f32 f32_clamp(f32 value, f32 lower, f32 upper)
{
    if (value < lower)
    {
        return lower;
    }
    else if (value > upper)
    {
        return upper;
    }
    else
    {
        return value;
    }
}

i32 i32_max(i32 a, i32 b) { return a > b ? a : b; }
i32 i32_min(i32 a, i32 b) { return a < b ? a : b; }
i32 i32_clamp(i32 value, i32 lower, i32 upper)
{
    if (value < lower)
    {
        return lower;
    }
    else if (value > upper)
    {
        return upper;
    }
    else
    {
        return value;
    }
}
