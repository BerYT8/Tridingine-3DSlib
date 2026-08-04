#pragma once

#include <stdbool.h>
#include <console/console.h>
#include <draw/2d/2d_shapes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Si este .c define ALLOCATE_SHMEM, se crean las variables.
// Si no lo define, se tratan como extern automáticos.
#ifdef ALLOCATE_SHMEM
  #define SHMEM_EXTD
  #define SHMEM_INITD(x) = x
#else
  #define SHMEM_EXTD extern
  #define SHMEM_INITD(x)
#endif

SHMEM_EXTD bool initialized SHMEM_INITD(false);

typedef struct D2D_Text
{
  u32 width;
  u32 height;
  bool drawed;
} D2D_Text;

#if defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

SHMEM_EXTD C2D_TextBuf consoleTopBuffer SHMEM_INITD(nullptr);
SHMEM_EXTD C2D_TextBuf consoleBotBuffer SHMEM_INITD(nullptr);
#endif

void D2D_InitTexts();
void D2D_TextsBegin();
void D2D_TextsEnd();
void D2D_TextsDeleteAllBuffers();

void InitConsoleBuffs();
void EndConsoleBuffs();
void ClearConsoleBuf(ScreenConsole console);
D2D_Text D2D_DrawText_Buf(
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

    D2D_WrapMode wrap,
    bool console,
    ScreenConsole consoleN
);

#ifdef __cplusplus
}
#endif
