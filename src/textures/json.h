#ifndef JSON_C_CODE_H
#define JSON_C_CODE_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;

typedef struct
{
    char *path;
    int x;
    int y;
} image;

typedef struct Tile
{
    float top;
    float left;
    float right;
    float bottom;
} Tile;

typedef struct Info
{
    int w;
    int h;
    u8 tiles;
    Tile *tilesValues;

    image **images;
    size_t imageCount;
} Info;
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(PLATFORM_PC)
    void CallText(const char *text);

    Info getJsonInfo(const char *path);
#endif

    void freeInfo(Info *info);

#ifdef __cplusplus
}
#endif

#endif