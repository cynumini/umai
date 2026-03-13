#pragma once

#include <stdlib.h>
#include <stdio.h>

#define SAKANA_ASSERT(expr)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            fprintf(stderr, "%s:%i:0: Assertion \"" #expr "\" failed.\n", __FILE__, __LINE__);                         \
            abort();                                                                                                   \
        }                                                                                                              \
    } while (false)

#define SAKANA_UNREACHABLE()                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stderr, "%s:%i:0: Unreachable.\n", __FILE__, __LINE__);                                                \
        abort();                                                                                                       \
    } while (false)

#define assert SAKANA_ASSERT
#define unreachable SAKANA_UNREACHABLE

namespace sakana
{
static_assert(sizeof(int) == 4);
using i32 = int;
static_assert(sizeof(long) == 8);
using i64 = long;
static_assert(sizeof(unsigned int) == 4);
using u32 = unsigned int;
static_assert(sizeof(unsigned long) == 8);
using u64 = long;
static_assert(sizeof(float) == 4);
using f32 = float;
static_assert(sizeof(double) == 8);
using f64 = double;
using usize = size_t;

template <typename T> T max(T a, T b)
{
    return a > b ? a : b;
}
} // namespace sakana
