#pragma once

#include <math.h>
#include "maths/vector2.h"
#include "maths/vector3.h"
#include <stdbool.h>

#define M_TAU (6.28318530717958647692528676655900576)

#define AngleToDegrees(_angle) ((_angle)*360.0f/M_TAU)

inline bool is_power_of_two(int n) {
    if (n <= 0) return false;
    
    return (n & (n - 1)) == 0;
}

inline float min2(float a, float b)
{
    float m = a;

    if (b < m) m = b;

    return m;
}

inline float max2(float a, float b)
{
    float m = a;

    if (b > m) m = b;

    return m;
}

inline float min3(float a, float b, float c)
{
    float m = a;

    if (b < m) m = b;
    if (c < m) m = c;

    return m;
}

inline float max3(float a, float b, float c)
{
    float m = a;

    if (b > m) m = b;
    if (c > m) m = c;

    return m;
}

inline float clampf(float value, float a, float b)
{
    float min = a < b ? a : b;
    float max = a > b ? a : b;

    if (value < min) return min;
    if (value > max) return max;

    return value;
}