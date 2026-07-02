#pragma once

#include "ints_defs.h"

typedef struct Color
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Color;

/* Constructors */

static inline Color Color_RGBA(
    u8 r,
    u8 g,
    u8 b,
    u8 a)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

static inline Color Color_New(void)
{
    return Color_RGBA(0, 0, 0, 0);
}

/* Assignment from uint32 */

typedef enum ColorFormat
{
    // RGBA family
    COLOR_RGBA8,   // 0xRRGGBBAA
    COLOR_RGB_8A,  // 0xRGB + A separado (equivalente conceptual)

    // ARGB family
    COLOR_ARGB8,   // 0xAARRGGBB

    // BGRA family
    COLOR_BGRA8,   // 0xBBGGRRAA
    COLOR_ABGR8,   // 0xAABBGGRR

    // Variants (engine-specific / uncommon but valid permutations)
    COLOR_RABG8,
    COLOR_RBGA8,
    COLOR_GRBA8,
    COLOR_GBRA8,
    COLOR_BARG8,
    COLOR_BRGA8,

    COLOR_FORMAT_COUNT
} ColorFormat;

#define COLOR_DEFAULT_FORMAT COLOR_ABGR8

static inline Color Color_FromUInt32(u32 c, ColorFormat fmt)
{
    Color out = {0};

    switch (fmt)
    {
        case COLOR_RGBA8:
            out.r = (c >> 24) & 0xFF;
            out.g = (c >> 16) & 0xFF;
            out.b = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        case COLOR_ARGB8:
            out.a = (c >> 24) & 0xFF;
            out.r = (c >> 16) & 0xFF;
            out.g = (c >> 8)  & 0xFF;
            out.b = (c)       & 0xFF;
            break;

        case COLOR_BGRA8:
            out.b = (c >> 24) & 0xFF;
            out.g = (c >> 16) & 0xFF;
            out.r = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        case COLOR_ABGR8:
            out.a = (c >> 24) & 0xFF;
            out.b = (c >> 16) & 0xFF;
            out.g = (c >> 8)  & 0xFF;
            out.r = (c)       & 0xFF;
            break;

        case COLOR_RABG8:
            out.r = (c >> 24) & 0xFF;
            out.a = (c >> 16) & 0xFF;
            out.b = (c >> 8)  & 0xFF;
            out.g = (c)       & 0xFF;
            break;

        case COLOR_RBGA8:
            out.r = (c >> 24) & 0xFF;
            out.b = (c >> 16) & 0xFF;
            out.g = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        case COLOR_GRBA8:
            out.g = (c >> 24) & 0xFF;
            out.r = (c >> 16) & 0xFF;
            out.b = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        case COLOR_GBRA8:
            out.g = (c >> 24) & 0xFF;
            out.b = (c >> 16) & 0xFF;
            out.r = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        case COLOR_BARG8:
            out.b = (c >> 24) & 0xFF;
            out.a = (c >> 16) & 0xFF;
            out.r = (c >> 8)  & 0xFF;
            out.g = (c)       & 0xFF;
            break;

        case COLOR_BRGA8:
            out.b = (c >> 24) & 0xFF;
            out.r = (c >> 16) & 0xFF;
            out.g = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;

        default:
            out.r = (c >> 24) & 0xFF;
            out.g = (c >> 16) & 0xFF;
            out.b = (c >> 8)  & 0xFF;
            out.a = (c)       & 0xFF;
            break;
    }

    return out;
}

static inline Color Color_FromUInt32_Default(u32 color)
{
    return Color_FromUInt32(color, COLOR_DEFAULT_FORMAT);
}

/* Convert to uint32 */

static inline u32 Color_ToUInt32(Color c, ColorFormat fmt)
{
    switch (fmt)
    {
        case COLOR_RGBA8:
            return ((u32)c.r << 24) | ((u32)c.g << 16) | ((u32)c.b << 8) | c.a;

        case COLOR_ARGB8:
            return ((u32)c.a << 24) | ((u32)c.r << 16) | ((u32)c.g << 8) | c.b;

        case COLOR_BGRA8:
            return ((u32)c.b << 24) | ((u32)c.g << 16) | ((u32)c.r << 8) | c.a;

        case COLOR_ABGR8:
            return ((u32)c.a << 24) | ((u32)c.b << 16) | ((u32)c.g << 8) | c.r;

        case COLOR_RABG8:
            return ((u32)c.r << 24) | ((u32)c.a << 16) | ((u32)c.b << 8) | c.g;

        case COLOR_RBGA8:
            return ((u32)c.r << 24) | ((u32)c.b << 16) | ((u32)c.g << 8) | c.a;

        case COLOR_GRBA8:
            return ((u32)c.g << 24) | ((u32)c.r << 16) | ((u32)c.b << 8) | c.a;

        case COLOR_GBRA8:
            return ((u32)c.g << 24) | ((u32)c.b << 16) | ((u32)c.r << 8) | c.a;

        case COLOR_BARG8:
            return ((u32)c.b << 24) | ((u32)c.a << 16) | ((u32)c.r << 8) | c.g;

        case COLOR_BRGA8:
            return ((u32)c.b << 24) | ((u32)c.r << 16) | ((u32)c.g << 8) | c.a;

        default:
            return ((u32)c.r << 24) | ((u32)c.g << 16) | ((u32)c.b << 8) | c.a;
    }
}

static inline u32 Color_ToUInt32_Default(Color color)
{
    return Color_ToUInt32(color, COLOR_DEFAULT_FORMAT);
}

/* Lerp */

static inline Color Color_Lerp(
    Color a,
    Color b,
    float t)
{
    return Color_RGBA(
        (a.r + t * (b.r - a.r)),
        (a.g + t * (b.g - a.g)),
        (a.b + t * (b.b - a.b)),
        (a.a + t * (b.a - a.a)));
}

/* MakeColor alias */

static inline Color Color_MakeColor(
    u8 r,
    u8 g,
    u8 b,
    u8 a)
{
    return Color_RGBA(r, g, b, a);
}

/* Constants */

static const Color Color_Red   = {255, 0,   0,   255};
static const Color Color_Blue  = {0,   0,   255, 255};
static const Color Color_Green = {0,   255, 0,   255};
static const Color Color_White = {255, 255, 255, 255};
static const Color Color_Black = {0,   0,   0,   255};