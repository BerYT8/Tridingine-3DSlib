#ifndef VEC2_H
#define VEC2_H

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdio.h>

/* Estructura Vec2 */
typedef struct
{
    float x;
    float y;
} Vec2;

/* Constructores */
static inline Vec2 vec2_create(float x, float y)
{
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline Vec2 vec2_zero()
{
    return vec2_create(0.0f, 0.0f);
}

/* Operadores aritméticos */
static inline Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return vec2_create(a.x + b.x, a.y + b.y);
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return vec2_create(a.x - b.x, a.y - b.y);
}

static inline Vec2 vec2_mul(Vec2 v, float scalar)
{
    return vec2_create(v.x * scalar, v.y * scalar);
}

static inline Vec2 vec2_div(Vec2 v, float scalar)
{
    return vec2_create(v.x / scalar, v.y / scalar);
}

/* Operadores compuestos */
static inline void vec2_add_assign(Vec2 *a, Vec2 b)
{
    a->x += b.x;
    a->y += b.y;
}

static inline void vec2_sub_assign(Vec2 *a, Vec2 b)
{
    a->x -= b.x;
    a->y -= b.y;
}

static inline void vec2_mul_assign(Vec2 *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
}

static inline void vec2_div_assign(Vec2 *v, float scalar)
{
    v->x /= scalar;
    v->y /= scalar;
}

/* Comparación */
static inline int vec2_equal(Vec2 a, Vec2 b)
{
    return a.x == b.x && a.y == b.y;
}

static inline int vec2_not_equal(Vec2 a, Vec2 b)
{
    return !vec2_equal(a, b);
}

/* Magnitud */
static inline float vec2_length(Vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline float vec2_length_squared(Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

/* Normalización */
static inline Vec2 vec2_normalized(Vec2 v)
{
    float len = vec2_length(v);

    if (len == 0.0f)
        return vec2_zero();

    return vec2_create(v.x / len, v.y / len);
}

static inline void vec2_normalize(Vec2 *v)
{
    float len = vec2_length(*v);

    if (len != 0.0f)
    {
        v->x /= len;
        v->y /= len;
    }
}

/* Producto escalar */
static inline float vec2_dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

/* Distancia */
static inline float vec2_distance(Vec2 a, Vec2 b)
{
    return vec2_length(vec2_sub(a, b));
}

/* Debug print */
static inline void vec2_print(Vec2 v)
{
    printf("(%.2f, %.2f)\n", v.x, v.y);
}

static inline float vec2_look_at(Vec2 origin, Vec2 target)
{
    float dx = target.x - origin.x;
    float dy = target.y - origin.y;

    float angle = atan2f(dx, -dy) * (180.0f / M_PI);

    if (angle < 0.0f)
        angle += 360.0f;

    return angle;
}

#endif /* VEC2_H */