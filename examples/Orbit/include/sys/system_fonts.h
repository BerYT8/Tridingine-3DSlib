#pragma once


#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

typedef struct D2D_Font D2D_Font;

const char *System_GetDefaultFontName();
D2D_Font *System_GetDefaultFont();

size_t System_GetFontsSize();

const char *System_GetFontNameByIndex(size_t index);
D2D_Font *System_GetFontByIndex(size_t index);

#ifdef __cplusplus
}
#endif