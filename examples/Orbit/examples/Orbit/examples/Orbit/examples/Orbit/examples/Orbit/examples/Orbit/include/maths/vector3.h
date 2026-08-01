#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdio.h>

/* Estructura Vec3 */
typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

/* Constructores */
static inline Vec3 vec3_create(float x, float y, float z)
{
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline Vec3 vec3_zero()
{
    return vec3_create(0.0f, 0.0f, 0.0f);
}

/* Operadores aritméticos */
static inline Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return vec3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return vec3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vec3 vec3_mul(Vec3 v, float scalar)
{
    return vec3_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

static inline Vec3 vec3_mul_vec(Vec3 v, Vec3 mul)
{
    return vec3_create(v.x * mul.x, v.y * mul.y, v.z * mul.z);
}

static inline Vec3 vec3_div(Vec3 v, float scalar)
{
    return vec3_create(v.x / scalar, v.y / scalar, v.z / scalar);
}

/* Operadores compuestos */
static inline void vec3_add_assign(Vec3 *a, Vec3 b)
{
    a->x += b.x;
    a->y += b.y;
    a->z += b.z;
}

static inline void vec3_sub_assign(Vec3 *a, Vec3 b)
{
    a->x -= b.x;
    a->y -= b.y;
    a->z -= b.z;
}

static inline void vec3_mul_assign(Vec3 *v, float scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
}

static inline void vec3_div_assign(Vec3 *v, float scalar)
{
    v->x /= scalar;
    v->y /= scalar;
    v->z /= scalar;
}

/* Comparación */
static inline int vec3_equal(Vec3 a, Vec3 b)
{
    return a.x == b.x &&
           a.y == b.y &&
           a.z == b.z;
}

static inline int vec3_not_equal(Vec3 a, Vec3 b)
{
    return !vec3_equal(a, b);
}

/* Magnitud */
static inline float vec3_length(Vec3 v)
{
    return sqrtf(v.x * v.x +
                 v.y * v.y +
                 v.z * v.z);
}

static inline float vec3_length_squared(Vec3 v)
{
    return v.x * v.x +
           v.y * v.y +
           v.z * v.z;
}

/* Normalización */
static inline Vec3 vec3_normalized(Vec3 v)
{
    float len = vec3_length(v);

    if (len == 0.0f)
        return vec3_zero();

    return vec3_create(
        v.x / len,
        v.y / len,
        v.z / len);
}

static inline void vec3_normalize(Vec3 *v)
{
    float len = vec3_length(*v);

    if (len != 0.0f)
    {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
}

/* Producto escalar */
static inline float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x +
           a.y * b.y +
           a.z * b.z;
}

/* Producto cruzado */
static inline Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return vec3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

/* Distancia */
static inline float vec3_distance(Vec3 a, Vec3 b)
{
    return vec3_length(vec3_sub(a, b));
}

/* Debug print */
static inline void vec3_print(Vec3 v)
{
    printf("(%.2f, %.2f, %.2f)\n",
           v.x, v.y, v.z);
}

static inline Vec3 vec3_clamp(Vec3 value, Vec3 a, Vec3 b)
{
    Vec3 v = value;
    float minax = a.x < b.x ? a.x : b.x;
    float maxax = a.x > b.x ? a.x : b.x;
    v.x = v.x < minax ? minax : (v.x > maxax ? maxax : v.x);
    float minay = a.y < b.y ? a.y : b.y;
    float maxay = a.y > b.y ? a.y : b.y;
    v.y = v.y < minay ? minay : (v.y > maxay ? maxay : v.y);
    float minaz = a.z < b.z ? a.z : b.z;
    float maxaz = a.z > b.z ? a.z : b.z;
    v.z = v.z < minaz ? minaz : (v.z > maxaz ? maxaz : v.z);
    return v;
}
static inline Vec3 vec3_clamp_float(Vec3 value, float a, float b)
{
    return vec3_clamp(value, vec3_create(a, a, a), vec3_create(b, b, b));
}

#endif /* VEC3_H */