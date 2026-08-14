#pragma once

#include <sys/system_fonts.h>

#include "../../color.h"

#include <stdbool.h>

typedef enum D2D_WrapMode
{
    LETTER_WRAP_MODE,
    WORD_WRAP_MODE,
    WRAP_NONE,
} D2D_WrapMode;

typedef enum D2D_Result
{
    D2D_NOT_INITIALIZED = -2,
    D2D_INVALID_ARGUMENT,
    D2D_ERROR,
    D2D_OK,
} D2D_Result;

#ifdef __cplusplus
extern "C"
{
#endif

bool D2D_Init();

void D2D_Prepare();

float D2D_ValueIsRotation(float value);
void D2D_AddRotation(float *value, float rotation);

D2D_Result D2D_DrawPoint(float x, float y, float rotation, float depth, float thickness, Color color);

D2D_Result D2D_DrawLine(float x0, float y0, Color c0,
                    float x1, float y1, Color c1,
                    float thickness, float rotation, float depth, float align);

D2D_Result D2D_DrawRectangle(float x, float y, float w, float h, float rotation, float depth, float alignX, float alignY, Color c1, Color c2, Color c3, Color c4);

static inline D2D_Result D2D_DrawRectSolid(float x, float y, float w, float h, float rotation, float depth, float alignX, float alignY, Color color)
{
    return D2D_DrawRectangle(x, y, w, h, rotation, depth, alignX, alignY, color, color, color, color);
};

// Borderer rects under development
/*
D2D_Result D2D_DrawBorderedRect(float x, float y, float w, float h, float radius, float depth, float alignX, float alignY, Color c1, Color c2, Color c3, Color c4);

static inline D2D_Result D2D_DrawBorderedRectSolid(float x, float y, float w, float h, float radius, float depth, float alignX, float alignY, Color color)
{
    return D2D_DrawBorderedRect(x, y, w, h, radius, depth, alignX, alignY, color, color, color, color);
};
*/

D2D_Result D2D_DrawTriangle(float x0, float y0, Color c0,
                        float x1, float y1, Color c1,
                        float x2, float y2, Color c2,
                        float rotation, float depth, float alignX, float alignY);

static inline D2D_Result D2D_DrawTriangleSolid(float x0, float y0,
                            float x1, float y1,
                            float x2, float y2,
                            float rotation, float depth, float alignX, float alignY, Color color)
{
    return D2D_DrawTriangle(x0, y0, color,
                            x1, y1, color,
                            x2, y2, color,
                            rotation, depth, alignX, alignY);
};

D2D_Result D2D_DrawEllipse(float x, float y, float radiusX, float radiusY, float rotation, float depth, float alignX, float alignY, Color c0, Color c1, Color c2, Color c3);

static D2D_Result D2D_DrawEllipseSolid(float x, float y, float radiusX, float radiusY, float rotation, float depth, float alignX, float alignY, Color color)
{
    return D2D_DrawEllipse(x, y, radiusX, radiusY, rotation, depth, alignX, alignY, color, color, color, color);
}

static D2D_Result D2D_DrawCircle(float x, float y, float radius, float rotation, float depth, float alignX, float alignY, Color c0, Color c1, Color c2, Color c3)
{
    return D2D_DrawEllipse(x, y, radius, radius, rotation, depth, alignX, alignY, c0, c1, c2, c3);
}

static D2D_Result D2D_DrawCircleSolid(float x, float y, float radius, float rotation, float depth, float alignX, float alignY, Color color)
{
    return D2D_DrawEllipseSolid(x, y, radius, radius, rotation, depth, alignX, alignY, color);
}

D2D_Font *D2D_OpenFont(const char* path);
void D2D_CloseFont(D2D_Font *font);

D2D_Result D2D_DrawText(
    const char* text,
    D2D_Font* font,
    float fontSize,
    Color color,

    float x,
    float y,
    float depth,
    float w,
    float h,

    float alignX,
    float alignY,

    float textAlignX,
    float textAlignY,

    float letterSpacing,
    float lineSpacing,

    D2D_WrapMode wrap
);

void D2D_Exit();

#ifdef __cplusplus
}
#endif