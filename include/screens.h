#pragma once

#include <maths/vector2.h>
#include <stdbool.h>

#define SCREEN_HEIGHT 240
#define SCREEN_TOP_WIDTH 400
#define SCREEN_BOT_WIDTH 320
#define SCREEN_GAP 20
#define SCREEN_BORDER 10

typedef enum S2S_Screen
{
    TOP,
    BOTTOM,
} S2S_Screen;

#ifdef __cplusplus
extern "C" {
#endif

bool PlatformPC();
bool Platform3DS();

bool S2S_IsCoverClosed();

void S2S_SetGamePaused(bool paused);

bool S2S_IsGamePaused();

bool S2S_ScreensInit();
void SetWindowTitle(const char* new_title);

void S2S_StopRunning();
bool S2S_ScreensRunning();

void S2S_SetCurrentScreen(S2S_Screen screen);
Vec2 S2S_GetScreenSize(S2S_Screen screen);

void S2S_BeginFrame();
void S2S_EndFrame();

void S2S_ScreensExit();

#ifdef __cplusplus
}
#endif
