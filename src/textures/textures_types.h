#pragma once

#include "json.h"
#include <textures/textures.h>

#include <ints_defs.h>

#if defined(PLATFORM_PC)
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#elif defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#endif

typedef struct T3DA_DrawSprite
{
    
    u8 tile;

    T3DA_AtlasTexture *atlas;

    // base size (tile size)
    float ix;
    float iy;

    float x;
    float y;

    float w;
    float h;

    // NEW: proper scale system
    float scaleX;
    float scaleY;

    float rotation;
    float depth;

    float alignX;
    float alignY;

#if defined(PLATFORM_PC)

    GLuint img;

    float u1;
    float v1;

    float u2;
    float v2;

#elif defined(PLATFORM_3DS)

    C2D_Image image;
    Tex3DS_SubTexture *subtex;
    // C2D_DrawParams params;
    C2D_ImageTint tint;

    bool changedTiles;

#endif

} T3DA_DrawSprite;

typedef struct T3DA_AtlasTexture
{
    char *path;
    u32 hash;

    int w;
    int h;

    u8 tiles;
    Tile *tilesValues;

#if defined(PLATFORM_PC)

    GLuint sheet;

#elif defined(PLATFORM_3DS)

    C2D_SpriteSheet sheet;
    C3D_Tex *tex;

#endif

} T3DA_AtlasTexture;


#ifdef __cplusplus
extern "C" 
{
#endif

void t3da_set_screen_size(u16 w, u16 h);

#ifdef __cplusplus
}
#endif